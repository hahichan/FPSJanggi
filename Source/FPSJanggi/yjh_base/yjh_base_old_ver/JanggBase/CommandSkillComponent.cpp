// Fill out your copyright notice in the Description page of Project Settings.

#include "CommandSkillComponent.h"
#include "BaseJanggiPiece.h"
#include "EnhancedInputComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

namespace
{
void ShowSkillDebug(UObject* WorldContextObject, const FString& Message, const FColor& Color = FColor::Yellow, float Duration = 2.0f)
{
	UE_LOG(LogTemp, Log, TEXT("[CommandSkillInput] %s"), *Message);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
	}
}

FString ExtractActionToken(const FString& ActionName)
{
	FString Token = ActionName;
	Token.RemoveFromStart(TEXT("IA_Command_"));
	Token.RemoveFromStart(TEXT("IA_"));
	return Token;
}

FString ExtractSkillToken(const FCommandSkillSpec& SkillSpec)
{
	if (!SkillSpec.SkillId.IsNone())
	{
		FString Token = SkillSpec.SkillId.ToString();
		Token.RemoveFromStart(TEXT("Cmd_"));
		Token.RemoveFromStart(TEXT("Command_"));
		return Token;
	}

	return SkillSpec.DisplayName.ToString().Replace(TEXT(" "), TEXT(""));
}

const TCHAR* ToSlotString(ECommandSkillSlot Slot)
{
	switch (Slot)
	{
	case ECommandSkillSlot::Slot1:
		return TEXT("Slot1");
	case ECommandSkillSlot::Slot2:
		return TEXT("Slot2");
	case ECommandSkillSlot::Slot3:
		return TEXT("Slot3");
	case ECommandSkillSlot::Slot4:
		return TEXT("Slot4");
	case ECommandSkillSlot::Slot5:
		return TEXT("Slot5");
	case ECommandSkillSlot::Slot6:
		return TEXT("Slot6");
	case ECommandSkillSlot::Slot7:
		return TEXT("Slot7");
	case ECommandSkillSlot::Slot8:
		return TEXT("Slot8");
	case ECommandSkillSlot::Slot9:
		return TEXT("Slot9");
	case ECommandSkillSlot::Slot10:
		return TEXT("Slot10");
	case ECommandSkillSlot::None:
	default:
		return TEXT("None");
	}
}

const TCHAR* ToSkillTypeString(ECommandSkillType SkillType)
{
	switch (SkillType)
	{
	case ECommandSkillType::Suicide:
		return TEXT("Suicide");
	case ECommandSkillType::KillEnemy:
		return TEXT("KillEnemy");
	case ECommandSkillType::ModifyMoveSpeed:
		return TEXT("ModifyMoveSpeed");
	case ECommandSkillType::ModifyAcceleration:
		return TEXT("ModifyAcceleration");
	case ECommandSkillType::None:
	default:
		return TEXT("None");
	}
}

const TCHAR* ToPieceStateString(EJanggiPieceState State)
{
	switch (State)
	{
	case EJanggiPieceState::Preparing:
		return TEXT("Preparing");
	case EJanggiPieceState::Fighting:
		return TEXT("Fighting");
	case EJanggiPieceState::Stunned:
		return TEXT("Stunned");
	case EJanggiPieceState::Dead:
		return TEXT("Dead");
	default:
		return TEXT("Unknown");
	}
}
}

UCommandSkillComponent::UCommandSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);

	SkillSlot1Action = nullptr;
	SkillSlot2Action = nullptr;
	SkillSlot3Action = nullptr;
	SkillSlot4Action = nullptr;
	SkillSlot5Action = nullptr;
	SkillSlot6Action = nullptr;
	SkillSlot7Action = nullptr;
	SkillSlot8Action = nullptr;
	SkillSlot9Action = nullptr;
	SkillSlot10Action = nullptr;
	SlotInputDebounceSeconds = 0.12f;
	bUseDefaultTestSkillSet = false;
	bEnableLegacyCommandWrappers = false;
}

void UCommandSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedOwnerPiece = Cast<ABaseJanggiPiece>(GetOwner());
	if (!CachedOwnerPiece.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] Owner is not ABaseJanggiPiece. Owner=%s"), *GetNameSafe(GetOwner()));
	}

	if (bUseDefaultTestSkillSet)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] bUseDefaultTestSkillSet is enabled, but auto-population is disabled to enforce explicit skill assignment."));
	}
	ValidateSkillSpecs();
	PrewarmActionSkillCache();
}

void UCommandSkillComponent::InitializeDefaultTestSkillsIfNeeded()
{
	if (!bUseDefaultTestSkillSet || Skills.Num() > 0)
	{
		return;
	}

	FCommandSkillSpec SuicideSpec;
	SuicideSpec.SkillId = TEXT("Cmd_Suicide");
	SuicideSpec.DisplayName = FText::FromString(TEXT("Suicide"));
	SuicideSpec.InputSlot = ECommandSkillSlot::Slot1;
	SuicideSpec.InputTag = TEXT("Command.Slot1");
	SuicideSpec.SkillType = ECommandSkillType::Suicide;
	SuicideSpec.bEnabled = true;
	SuicideSpec.Magnitude = 99999.0f;
	Skills.Add(SuicideSpec);

	FCommandSkillSpec KillEnemySpec;
	KillEnemySpec.SkillId = TEXT("Cmd_KillEnemy");
	KillEnemySpec.DisplayName = FText::FromString(TEXT("Kill Enemy"));
	KillEnemySpec.InputSlot = ECommandSkillSlot::Slot2;
	KillEnemySpec.InputTag = TEXT("Command.Slot2");
	KillEnemySpec.SkillType = ECommandSkillType::KillEnemy;
	KillEnemySpec.bEnabled = true;
	KillEnemySpec.Magnitude = 99999.0f;
	Skills.Add(KillEnemySpec);

	FCommandSkillSpec SpeedSpec;
	SpeedSpec.SkillId = TEXT("Cmd_ModifyMoveSpeed");
	SpeedSpec.DisplayName = FText::FromString(TEXT("Modify Move Speed"));
	SpeedSpec.InputSlot = ECommandSkillSlot::Slot3;
	SpeedSpec.InputTag = TEXT("Command.Slot3");
	SpeedSpec.SkillType = ECommandSkillType::ModifyMoveSpeed;
	SpeedSpec.bEnabled = true;
	SpeedSpec.Magnitude = 900.0f;
	Skills.Add(SpeedSpec);

	FCommandSkillSpec AccelSpec;
	AccelSpec.SkillId = TEXT("Cmd_ModifyAcceleration");
	AccelSpec.DisplayName = FText::FromString(TEXT("Modify Acceleration"));
	AccelSpec.InputSlot = ECommandSkillSlot::Slot4;
	AccelSpec.InputTag = TEXT("Command.Slot4");
	AccelSpec.SkillType = ECommandSkillType::ModifyAcceleration;
	AccelSpec.bEnabled = true;
	AccelSpec.Magnitude = 4096.0f;
	AccelSpec.SecondaryMagnitude = 2048.0f;
	Skills.Add(AccelSpec);
}

void UCommandSkillComponent::PrewarmActionSkillCache()
{
	ActionNameToRuntimeSkillIdCache.Reset();
	UnmatchedActionNameCache.Reset();

	TArray<const UInputAction*> CandidateActions;
	CandidateActions.Reserve(20);

	auto AddActionIfValid = [&CandidateActions](const TObjectPtr<UInputAction>& InputAction)
	{
		if (IsValid(InputAction))
		{
			CandidateActions.Add(InputAction.Get());
		}
	};

	AddActionIfValid(SkillSlot1Action);
	AddActionIfValid(SkillSlot2Action);
	AddActionIfValid(SkillSlot3Action);
	AddActionIfValid(SkillSlot4Action);
	AddActionIfValid(SkillSlot5Action);
	AddActionIfValid(SkillSlot6Action);
	AddActionIfValid(SkillSlot7Action);
	AddActionIfValid(SkillSlot8Action);
	AddActionIfValid(SkillSlot9Action);
	AddActionIfValid(SkillSlot10Action);

	for (const UInputAction* InputAction : CandidateActions)
	{
		FindSkillByAction(InputAction);
	}

	UE_LOG(LogTemp, Log, TEXT("[UCommandSkillComponent] Action-skill cache prewarmed. Matched=%d Unmatched=%d"), ActionNameToRuntimeSkillIdCache.Num(), UnmatchedActionNameCache.Num());
}

void UCommandSkillComponent::BindInput(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (!IsValid(EnhancedInputComponent))
	{
		return;
	}

	auto BindSkillAction = [EnhancedInputComponent, this](UInputAction* Action, void (UCommandSkillComponent::*Handler)(), const TCHAR* SlotLabel)
	{
		if (!IsValid(Action))
		{
			UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] BindInput skipped: %s action is not assigned."), SlotLabel);
			return;
		}

		Action->bConsumeInput = false;
		EnhancedInputComponent->BindAction(Action, ETriggerEvent::Started, this, Handler);
		EnhancedInputComponent->BindAction(Action, ETriggerEvent::Triggered, this, Handler);
		UE_LOG(LogTemp, Log, TEXT("[UCommandSkillComponent] Bound %s to action=%s (Started+Triggered, consume=false)"), SlotLabel, *GetNameSafe(Action));
	};

	BindSkillAction(SkillSlot1Action, &UCommandSkillComponent::OnSkillSlot1Triggered, TEXT("Slot1"));
	BindSkillAction(SkillSlot2Action, &UCommandSkillComponent::OnSkillSlot2Triggered, TEXT("Slot2"));
	BindSkillAction(SkillSlot3Action, &UCommandSkillComponent::OnSkillSlot3Triggered, TEXT("Slot3"));
	BindSkillAction(SkillSlot4Action, &UCommandSkillComponent::OnSkillSlot4Triggered, TEXT("Slot4"));
	BindSkillAction(SkillSlot5Action, &UCommandSkillComponent::OnSkillSlot5Triggered, TEXT("Slot5"));
	BindSkillAction(SkillSlot6Action, &UCommandSkillComponent::OnSkillSlot6Triggered, TEXT("Slot6"));
	BindSkillAction(SkillSlot7Action, &UCommandSkillComponent::OnSkillSlot7Triggered, TEXT("Slot7"));
	BindSkillAction(SkillSlot8Action, &UCommandSkillComponent::OnSkillSlot8Triggered, TEXT("Slot8"));
	BindSkillAction(SkillSlot9Action, &UCommandSkillComponent::OnSkillSlot9Triggered, TEXT("Slot9"));
	BindSkillAction(SkillSlot10Action, &UCommandSkillComponent::OnSkillSlot10Triggered, TEXT("Slot10"));
}

bool UCommandSkillComponent::ShouldProcessSlotInput(ECommandSkillSlot Slot)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return true;
	}

	const float Now = World->GetTimeSeconds();
	float& LastTime = LastSlotInputTime.FindOrAdd(Slot);
	if (LastTime > 0.0f && (Now - LastTime) < SlotInputDebounceSeconds)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[UCommandSkillComponent] Debounced input for %s."), ToSlotString(Slot));
		return false;
	}

	LastTime = Now;
	return true;
}

void UCommandSkillComponent::TryUseSuicideCommand()
{
	if (!bEnableLegacyCommandWrappers)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] Legacy command wrapper disabled: TryUseSuicideCommand"));
		return;
	}

	if (const FCommandSkillSpec* SkillSpec = FindFirstSkillByType(ECommandSkillType::Suicide))
	{
		if (SkillSpec->InputSlot != ECommandSkillSlot::None)
		{
			TryUseSkillBySlot(SkillSpec->InputSlot);
		}
		else
		{
			TryUseSkillByTag(SkillSpec->InputTag);
		}
	}
}

void UCommandSkillComponent::TryUseKillEnemyCommand()
{
	if (!bEnableLegacyCommandWrappers)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] Legacy command wrapper disabled: TryUseKillEnemyCommand"));
		return;
	}

	if (const FCommandSkillSpec* SkillSpec = FindFirstSkillByType(ECommandSkillType::KillEnemy))
	{
		if (SkillSpec->InputSlot != ECommandSkillSlot::None)
		{
			TryUseSkillBySlot(SkillSpec->InputSlot);
		}
		else
		{
			TryUseSkillByTag(SkillSpec->InputTag);
		}
	}
}

void UCommandSkillComponent::TryUseSpeedBuffCommand()
{
	if (!bEnableLegacyCommandWrappers)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] Legacy command wrapper disabled: TryUseSpeedBuffCommand"));
		return;
	}

	if (const FCommandSkillSpec* SkillSpec = FindFirstSkillByType(ECommandSkillType::ModifyMoveSpeed))
	{
		if (SkillSpec->InputSlot != ECommandSkillSlot::None)
		{
			TryUseSkillBySlot(SkillSpec->InputSlot);
		}
		else
		{
			TryUseSkillByTag(SkillSpec->InputTag);
		}
	}
}

void UCommandSkillComponent::TryUseAccelBuffCommand()
{
	if (!bEnableLegacyCommandWrappers)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] Legacy command wrapper disabled: TryUseAccelBuffCommand"));
		return;
	}

	if (const FCommandSkillSpec* SkillSpec = FindFirstSkillByType(ECommandSkillType::ModifyAcceleration))
	{
		if (SkillSpec->InputSlot != ECommandSkillSlot::None)
		{
			TryUseSkillBySlot(SkillSpec->InputSlot);
		}
		else
		{
			TryUseSkillByTag(SkillSpec->InputTag);
		}
	}
}

void UCommandSkillComponent::TryUseSkillBySlot(ECommandSkillSlot Slot)
{
	ShowSkillDebug(this, FString::Printf(TEXT("Input: %s pressed"), ToSlotString(Slot)), FColor::Cyan, 1.2f);

	const FCommandSkillSpec* SkillSpec = FindSkillBySlot(Slot);
	if (!SkillSpec)
	{
		ShowSkillDebug(this, FString::Printf(TEXT("No skill in %s"), ToSlotString(Slot)), FColor::Red, 2.5f);
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] No skill mapped to slot %s. Owner=%s"), ToSlotString(Slot), *GetNameSafe(ResolveOwnerPiece()));
		return;
	}

	FString Reason;
	if (!CanExecuteSkillSpec(*SkillSpec, Reason))
	{
		ShowSkillDebug(this, FString::Printf(TEXT("Blocked: %s (%s)"), *SkillSpec->SkillId.ToString(), *Reason), FColor::Red, 2.5f);
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] Skill blocked. Id=%s Slot=%s Reason=%s"), *SkillSpec->SkillId.ToString(), ToSlotString(Slot), *Reason);
		return;
	}

	ABaseJanggiPiece* OwnerPiece = ResolveOwnerPiece();
	if (!IsValid(OwnerPiece))
	{
		return;
	}

	const FName RuntimeId = ResolveSkillRuntimeId(*SkillSpec);
	if (OwnerPiece->HasAuthority())
	{
		ServerUseSkillById_Implementation(RuntimeId);
	}
	else
	{
		ServerUseSkillById(RuntimeId);
	}

	ShowSkillDebug(this, FString::Printf(TEXT("Cast request: %s via %s"), *SkillSpec->SkillId.ToString(), ToSlotString(Slot)), FColor::Green, 2.0f);
}

void UCommandSkillComponent::TryUseSkillByTag(FName InputTag)
{
	if (InputTag.IsNone())
	{
		return;
	}

	const FCommandSkillSpec* SkillSpec = FindSkillByTag(InputTag);
	if (!SkillSpec)
	{
		ShowSkillDebug(this, FString::Printf(TEXT("No skill mapped to InputTag=%s"), *InputTag.ToString()), FColor::Red, 2.5f);
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] No skill mapped to InputTag=%s. Owner=%s"), *InputTag.ToString(), *GetNameSafe(ResolveOwnerPiece()));
		return;
	}

	FString Reason;
	if (!CanExecuteSkillSpec(*SkillSpec, Reason))
	{
		ShowSkillDebug(this, FString::Printf(TEXT("Blocked: %s (%s)"), *SkillSpec->SkillId.ToString(), *Reason), FColor::Red, 2.5f);
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] Skill blocked. Id=%s Tag=%s Reason=%s"), *SkillSpec->SkillId.ToString(), *InputTag.ToString(), *Reason);
		return;
	}

	ABaseJanggiPiece* OwnerPiece = ResolveOwnerPiece();
	if (!IsValid(OwnerPiece))
	{
		return;
	}

	const FName RuntimeId = ResolveSkillRuntimeId(*SkillSpec);
	if (OwnerPiece->HasAuthority())
	{
		ServerUseSkillById_Implementation(RuntimeId);
	}
	else
	{
		ServerUseSkillById(RuntimeId);
	}

	ShowSkillDebug(this, FString::Printf(TEXT("Cast request: %s via Tag=%s"), *SkillSpec->SkillId.ToString(), *InputTag.ToString()), FColor::Green, 2.0f);
}

void UCommandSkillComponent::ServerUseSkillById_Implementation(FName SkillRuntimeId)
{
	ABaseJanggiPiece* OwnerPiece = ResolveOwnerPiece();
	if (!IsValid(OwnerPiece) || !OwnerPiece->HasAuthority())
	{
		return;
	}

	const FCommandSkillSpec* SkillSpec = FindSkillByRuntimeId(SkillRuntimeId);
	if (!SkillSpec)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] ServerUseSkillById failed: unknown id=%s"), *SkillRuntimeId.ToString());
		return;
	}

	FString Reason;
	if (!CanExecuteSkillSpec(*SkillSpec, Reason))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] Server blocked skill. Id=%s Reason=%s"), *SkillSpec->SkillId.ToString(), *Reason);
		return;
	}

	const bool bExecuted = ExecuteSkillSpecServer(*SkillSpec);
	if (!bExecuted)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] ExecuteSkillSpecServer returned false. Id=%s Type=%s"), *SkillSpec->SkillId.ToString(), ToSkillTypeString(SkillSpec->SkillType));
		return;
	}

	if (SkillSpec->Cooldown > 0.0f)
	{
		LastSkillUseServerTime.FindOrAdd(SkillSpec->SkillId) = GetWorld()->GetTimeSeconds();
	}
}

const FCommandSkillSpec* UCommandSkillComponent::FindSkillByRuntimeId(FName SkillRuntimeId) const
{
	if (SkillRuntimeId.IsNone())
	{
		return nullptr;
	}

	for (const FCommandSkillSpec& SkillSpec : Skills)
	{
		if (ResolveSkillRuntimeId(SkillSpec) == SkillRuntimeId)
		{
			return &SkillSpec;
		}
	}

	return nullptr;
}

ABaseJanggiPiece* UCommandSkillComponent::ResolveOwnerPiece() const
{
	if (CachedOwnerPiece.IsValid())
	{
		return CachedOwnerPiece.Get();
	}

	return Cast<ABaseJanggiPiece>(GetOwner());
}

ABaseJanggiPiece* UCommandSkillComponent::ResolveEnemyTarget() const
{
	ABaseJanggiPiece* OwnerPiece = ResolveOwnerPiece();
	if (!IsValid(OwnerPiece))
	{
		return nullptr;
	}

	TArray<AActor*> FoundPieces;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseJanggiPiece::StaticClass(), FoundPieces);

	ABaseJanggiPiece* FallbackTarget = nullptr;
	for (AActor* Actor : FoundPieces)
	{
		ABaseJanggiPiece* Candidate = Cast<ABaseJanggiPiece>(Actor);
		if (!IsValid(Candidate) || Candidate == OwnerPiece)
		{
			continue;
		}

		if (Candidate->GetCharacterState() == EJanggiPieceState::Dead)
		{
			continue;
		}

		if (!IsValid(FallbackTarget))
		{
			FallbackTarget = Candidate;
		}

		const bool bDifferentFaction = (OwnerPiece->GetFaction() != EJanggiFaction::None) && (Candidate->GetFaction() != EJanggiFaction::None) && (OwnerPiece->GetFaction() != Candidate->GetFaction());
		if (bDifferentFaction)
		{
			return Candidate;
		}
	}

	return FallbackTarget;
}

const FCommandSkillSpec* UCommandSkillComponent::FindSkillBySlot(ECommandSkillSlot Slot) const
{
	if (Slot == ECommandSkillSlot::None)
	{
		return nullptr;
	}

	for (const FCommandSkillSpec& SkillSpec : Skills)
	{
		if (!SkillSpec.bEnabled)
		{
			continue;
		}

		if (SkillSpec.InputSlot == Slot)
		{
			return &SkillSpec;
		}
	}

	return nullptr;
}

const FCommandSkillSpec* UCommandSkillComponent::FindSkillByTag(FName InputTag) const
{
	if (InputTag.IsNone())
	{
		return nullptr;
	}

	for (const FCommandSkillSpec& SkillSpec : Skills)
	{
		if (!SkillSpec.bEnabled)
		{
			continue;
		}

		if (SkillSpec.InputTag == InputTag)
		{
			return &SkillSpec;
		}
	}

	return nullptr;
}

const FCommandSkillSpec* UCommandSkillComponent::FindFirstSkillByType(ECommandSkillType SkillType) const
{
	for (const FCommandSkillSpec& SkillSpec : Skills)
	{
		if (!SkillSpec.bEnabled)
		{
			continue;
		}

		if (SkillSpec.SkillType == SkillType)
		{
			return &SkillSpec;
		}
	}

	return nullptr;
}

FName UCommandSkillComponent::ResolveSkillRuntimeId(const FCommandSkillSpec& SkillSpec) const
{
	if (!SkillSpec.SkillId.IsNone())
	{
		return SkillSpec.SkillId;
	}

	return FName(*FString::Printf(TEXT("Auto_%d_%s"), static_cast<int32>(SkillSpec.SkillType), ToSlotString(SkillSpec.InputSlot)));
}

bool UCommandSkillComponent::IsSkillOnCooldown(const FCommandSkillSpec& SkillSpec, float Now, float& OutRemaining) const
{
	OutRemaining = 0.0f;

	if (SkillSpec.Cooldown <= 0.0f)
	{
		return false;
	}

	const float* LastUseTime = LastSkillUseServerTime.Find(SkillSpec.SkillId);
	if (!LastUseTime)
	{
		return false;
	}

	const float Elapsed = Now - *LastUseTime;
	if (Elapsed >= SkillSpec.Cooldown)
	{
		return false;
	}

	OutRemaining = SkillSpec.Cooldown - Elapsed;
	return true;
}

bool UCommandSkillComponent::CanExecuteSkillSpec(const FCommandSkillSpec& SkillSpec, FString& OutReason) const
{
	OutReason = TEXT("OK");

	ABaseJanggiPiece* OwnerPiece = ResolveOwnerPiece();
	if (!IsValid(OwnerPiece))
	{
		OutReason = TEXT("OwnerPieceInvalid");
		return false;
	}

	if (!SkillSpec.bEnabled)
	{
		OutReason = TEXT("SkillDisabled");
		return false;
	}

	if (SkillSpec.SkillType == ECommandSkillType::None)
	{
		OutReason = TEXT("SkillTypeNone");
		return false;
	}

	if (SkillSpec.bUseAllowedStates)
	{
		if (!SkillSpec.AllowedStates.Contains(OwnerPiece->GetCharacterState()))
		{
			OutReason = FString::Printf(TEXT("StateBlocked(%s)"), ToPieceStateString(OwnerPiece->GetCharacterState()));
			return false;
		}
	}
	else if (!OwnerPiece->CanUseSkill())
	{
		OutReason = FString::Printf(TEXT("CanUseSkillFalse(%s)"), ToPieceStateString(OwnerPiece->GetCharacterState()));
		return false;
	}

	if (OwnerPiece->HasAuthority())
	{
		float RemainingCooldown = 0.0f;
		if (IsSkillOnCooldown(SkillSpec, GetWorld()->GetTimeSeconds(), RemainingCooldown))
		{
			OutReason = FString::Printf(TEXT("Cooldown(%.2f)"), RemainingCooldown);
			return false;
		}
	}

	return true;
}

bool UCommandSkillComponent::CanExecuteCommand(const TCHAR* CommandName) const
{
	ABaseJanggiPiece* OwnerPiece = ResolveOwnerPiece();
	if (!IsValid(OwnerPiece))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] %s command failed: owner piece is invalid."), CommandName);
		return false;
	}

	if (!OwnerPiece->CanUseSkill())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] %s command blocked by state. User=%s State=%s"), CommandName, *GetNameSafe(OwnerPiece), ToPieceStateString(OwnerPiece->GetCharacterState()));
		return false;
	}

	return true;
}

bool UCommandSkillComponent::ExecuteSkillSpecServer(const FCommandSkillSpec& SkillSpec)
{
	switch (SkillSpec.SkillType)
	{
	case ECommandSkillType::Suicide:
		return ExecuteSuicideSkillServer(SkillSpec);
	case ECommandSkillType::KillEnemy:
		return ExecuteKillEnemySkillServer(SkillSpec);
	case ECommandSkillType::ModifyMoveSpeed:
		return ExecuteModifyMoveSpeedSkillServer(SkillSpec);
	case ECommandSkillType::ModifyAcceleration:
		return ExecuteModifyAccelerationSkillServer(SkillSpec);
	case ECommandSkillType::None:
	default:
		return false;
	}
}

bool UCommandSkillComponent::ExecuteSuicideSkillServer(const FCommandSkillSpec& SkillSpec)
{
	ABaseJanggiPiece* OwnerPiece = ResolveOwnerPiece();
	if (!IsValid(OwnerPiece) || !OwnerPiece->HasAuthority())
	{
		return false;
	}

	AController* InstigatorController = OwnerPiece->GetController();
	UE_LOG(LogTemp, Log, TEXT("[UCommandSkillComponent] ExecuteSkill Suicide: SkillId=%s User=%s Damage=%.1f"), *SkillSpec.SkillId.ToString(), *GetNameSafe(OwnerPiece), SkillSpec.Magnitude);

	FDamageEvent DamageEvent;
	OwnerPiece->TakeDamage(SkillSpec.Magnitude, DamageEvent, InstigatorController, OwnerPiece);
	return true;
}

bool UCommandSkillComponent::ExecuteKillEnemySkillServer(const FCommandSkillSpec& SkillSpec)
{
	ABaseJanggiPiece* OwnerPiece = ResolveOwnerPiece();
	if (!IsValid(OwnerPiece) || !OwnerPiece->HasAuthority())
	{
		return false;
	}

	ABaseJanggiPiece* TargetPiece = ResolveEnemyTarget();
	if (!IsValid(TargetPiece))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] ExecuteSkill KillEnemy failed: target not found. SkillId=%s User=%s"), *SkillSpec.SkillId.ToString(), *GetNameSafe(OwnerPiece));
		return false;
	}

	AController* InstigatorController = OwnerPiece->GetController();
	UE_LOG(LogTemp, Log, TEXT("[UCommandSkillComponent] ExecuteSkill KillEnemy: SkillId=%s User=%s Target=%s Damage=%.1f"), *SkillSpec.SkillId.ToString(), *GetNameSafe(OwnerPiece), *GetNameSafe(TargetPiece), SkillSpec.Magnitude);

	FDamageEvent DamageEvent;
	TargetPiece->TakeDamage(SkillSpec.Magnitude, DamageEvent, InstigatorController, OwnerPiece);
	return true;
}

bool UCommandSkillComponent::ExecuteModifyMoveSpeedSkillServer(const FCommandSkillSpec& SkillSpec)
{
	ABaseJanggiPiece* OwnerPiece = ResolveOwnerPiece();
	if (!IsValid(OwnerPiece) || !OwnerPiece->HasAuthority())
	{
		return false;
	}

	UCharacterMovementComponent* MoveComp = OwnerPiece->GetCharacterMovement();
	if (!IsValid(MoveComp))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] ExecuteSkill ModifyMoveSpeed failed: MoveComp invalid. SkillId=%s User=%s"), *SkillSpec.SkillId.ToString(), *GetNameSafe(OwnerPiece));
		return false;
	}

	const float OldSpeed = MoveComp->MaxWalkSpeed;
	MoveComp->MaxWalkSpeed = SkillSpec.Magnitude;

	UE_LOG(LogTemp, Log, TEXT("[UCommandSkillComponent] ExecuteSkill ModifyMoveSpeed: SkillId=%s User=%s MaxWalkSpeed %.1f -> %.1f"), *SkillSpec.SkillId.ToString(), *GetNameSafe(OwnerPiece), OldSpeed, MoveComp->MaxWalkSpeed);
	return true;
}

bool UCommandSkillComponent::ExecuteModifyAccelerationSkillServer(const FCommandSkillSpec& SkillSpec)
{
	ABaseJanggiPiece* OwnerPiece = ResolveOwnerPiece();
	if (!IsValid(OwnerPiece) || !OwnerPiece->HasAuthority())
	{
		return false;
	}

	UCharacterMovementComponent* MoveComp = OwnerPiece->GetCharacterMovement();
	if (!IsValid(MoveComp))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] ExecuteSkill ModifyAcceleration failed: MoveComp invalid. SkillId=%s User=%s"), *SkillSpec.SkillId.ToString(), *GetNameSafe(OwnerPiece));
		return false;
	}

	const float OldAcceleration = MoveComp->MaxAcceleration;
	const float OldBraking = MoveComp->BrakingDecelerationWalking;

	MoveComp->MaxAcceleration = SkillSpec.Magnitude;
	MoveComp->BrakingDecelerationWalking = SkillSpec.SecondaryMagnitude;

	UE_LOG(LogTemp, Log, TEXT("[UCommandSkillComponent] ExecuteSkill ModifyAcceleration: SkillId=%s User=%s MaxAcceleration %.1f -> %.1f, BrakingDecel %.1f -> %.1f"), *SkillSpec.SkillId.ToString(), *GetNameSafe(OwnerPiece), OldAcceleration, MoveComp->MaxAcceleration, OldBraking, MoveComp->BrakingDecelerationWalking);
	return true;
}

void UCommandSkillComponent::OnSkillSlot1Triggered()
{
	if (!ShouldProcessSlotInput(ECommandSkillSlot::Slot1))
	{
		return;
	}

	ShowSkillDebug(this, TEXT("Input detected: Slot1"), FColor::Cyan, 1.2f);
	const UInputAction* ActiveAction = SkillSlot1Action.Get();
	TryUseSkillByAssignedAction(ActiveAction, ECommandSkillSlot::Slot1);
}

void UCommandSkillComponent::OnSkillSlot2Triggered()
{
	if (!ShouldProcessSlotInput(ECommandSkillSlot::Slot2))
	{
		return;
	}

	ShowSkillDebug(this, TEXT("Input detected: Slot2"), FColor::Cyan, 1.2f);
	const UInputAction* ActiveAction = SkillSlot2Action.Get();
	TryUseSkillByAssignedAction(ActiveAction, ECommandSkillSlot::Slot2);
}

void UCommandSkillComponent::OnSkillSlot3Triggered()
{
	if (!ShouldProcessSlotInput(ECommandSkillSlot::Slot3))
	{
		return;
	}

	ShowSkillDebug(this, TEXT("Input detected: Slot3"), FColor::Cyan, 1.2f);
	const UInputAction* ActiveAction = SkillSlot3Action.Get();
	TryUseSkillByAssignedAction(ActiveAction, ECommandSkillSlot::Slot3);
}

void UCommandSkillComponent::OnSkillSlot4Triggered()
{
	if (!ShouldProcessSlotInput(ECommandSkillSlot::Slot4))
	{
		return;
	}

	ShowSkillDebug(this, TEXT("Input detected: Slot4"), FColor::Cyan, 1.2f);
	const UInputAction* ActiveAction = SkillSlot4Action.Get();
	TryUseSkillByAssignedAction(ActiveAction, ECommandSkillSlot::Slot4);
}

void UCommandSkillComponent::OnSkillSlot5Triggered()
{
	if (!ShouldProcessSlotInput(ECommandSkillSlot::Slot5))
	{
		return;
	}

	ShowSkillDebug(this, TEXT("Input detected: Slot5"), FColor::Cyan, 1.2f);
	const UInputAction* ActiveAction = SkillSlot5Action.Get();
	TryUseSkillByAssignedAction(ActiveAction, ECommandSkillSlot::Slot5);
}

void UCommandSkillComponent::OnSkillSlot6Triggered()
{
	if (!ShouldProcessSlotInput(ECommandSkillSlot::Slot6))
	{
		return;
	}

	ShowSkillDebug(this, TEXT("Input detected: Slot6"), FColor::Cyan, 1.2f);
	const UInputAction* ActiveAction = SkillSlot6Action.Get();
	TryUseSkillByAssignedAction(ActiveAction, ECommandSkillSlot::Slot6);
}

void UCommandSkillComponent::OnSkillSlot7Triggered()
{
	if (!ShouldProcessSlotInput(ECommandSkillSlot::Slot7))
	{
		return;
	}

	ShowSkillDebug(this, TEXT("Input detected: Slot7"), FColor::Cyan, 1.2f);
	const UInputAction* ActiveAction = SkillSlot7Action.Get();
	TryUseSkillByAssignedAction(ActiveAction, ECommandSkillSlot::Slot7);
}

void UCommandSkillComponent::OnSkillSlot8Triggered()
{
	if (!ShouldProcessSlotInput(ECommandSkillSlot::Slot8))
	{
		return;
	}

	ShowSkillDebug(this, TEXT("Input detected: Slot8"), FColor::Cyan, 1.2f);
	const UInputAction* ActiveAction = SkillSlot8Action.Get();
	TryUseSkillByAssignedAction(ActiveAction, ECommandSkillSlot::Slot8);
}

void UCommandSkillComponent::OnSkillSlot9Triggered()
{
	if (!ShouldProcessSlotInput(ECommandSkillSlot::Slot9))
	{
		return;
	}

	ShowSkillDebug(this, TEXT("Input detected: Slot9"), FColor::Cyan, 1.2f);
	const UInputAction* ActiveAction = SkillSlot9Action.Get();
	TryUseSkillByAssignedAction(ActiveAction, ECommandSkillSlot::Slot9);
}

void UCommandSkillComponent::OnSkillSlot10Triggered()
{
	if (!ShouldProcessSlotInput(ECommandSkillSlot::Slot10))
	{
		return;
	}

	ShowSkillDebug(this, TEXT("Input detected: Slot10"), FColor::Cyan, 1.2f);
	const UInputAction* ActiveAction = SkillSlot10Action.Get();
	TryUseSkillByAssignedAction(ActiveAction, ECommandSkillSlot::Slot10);
}

const FCommandSkillSpec* UCommandSkillComponent::FindSkillByAction(const UInputAction* InputAction) const
{
	if (!IsValid(InputAction))
	{
		return nullptr;
	}

	UCommandSkillComponent* MutableThis = const_cast<UCommandSkillComponent*>(this);
	const FName ActionName = InputAction->GetFName();
	if (const FName* CachedRuntimeId = ActionNameToRuntimeSkillIdCache.Find(ActionName))
	{
		return FindSkillByRuntimeId(*CachedRuntimeId);
	}

	if (UnmatchedActionNameCache.Contains(ActionName))
	{
		return nullptr;
	}

	const FString ActionToken = ExtractActionToken(InputAction->GetName());
	if (ActionToken.IsEmpty())
	{
		MutableThis->UnmatchedActionNameCache.Add(ActionName);
		return nullptr;
	}

	for (const FCommandSkillSpec& SkillSpec : Skills)
	{
		if (!SkillSpec.bEnabled)
		{
			continue;
		}

		const FString SkillToken = ExtractSkillToken(SkillSpec);
		if (!SkillToken.IsEmpty() && ActionToken.Contains(SkillToken, ESearchCase::IgnoreCase))
		{
			MutableThis->ActionNameToRuntimeSkillIdCache.Add(ActionName, ResolveSkillRuntimeId(SkillSpec));
			return &SkillSpec;
		}
	}

	MutableThis->UnmatchedActionNameCache.Add(ActionName);

	return nullptr;
}

void UCommandSkillComponent::TryUseSkillByAssignedAction(const UInputAction* InputAction, ECommandSkillSlot FallbackSlot)
{
	ABaseJanggiPiece* OwnerPiece = ResolveOwnerPiece();
	const FName InputActionName = IsValid(InputAction) ? InputAction->GetFName() : NAME_None;
	UE_LOG(LogTemp, Log, TEXT("[UCommandSkillComponent] Input received. Owner=%s Slot=%s Action=%s"), *GetNameSafe(OwnerPiece), ToSlotString(FallbackSlot), *InputActionName.ToString());

	if (!IsValid(InputAction))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] Ignored input: slot action is not assigned. Slot=%s Owner=%s"), ToSlotString(FallbackSlot), *GetNameSafe(OwnerPiece));
		return;
	}

	const FCommandSkillSpec* SkillToExecute = FindSkillBySlot(FallbackSlot);
	const FName RuntimeId = SkillToExecute ? ResolveSkillRuntimeId(*SkillToExecute) : NAME_None;

	OnCommandSkillInputReceived.Broadcast(static_cast<int32>(FallbackSlot), InputActionName, RuntimeId);
	if (IsValid(OwnerPiece))
	{
		OwnerPiece->OnCommandSkillInputEvent(static_cast<int32>(FallbackSlot), InputActionName, RuntimeId);
	}

	if (!SkillToExecute)
	{
		UE_LOG(LogTemp, Log, TEXT("[UCommandSkillComponent] Ignored input: no SkillSpec assigned to slot. Slot=%s Action=%s Owner=%s"), ToSlotString(FallbackSlot), *InputActionName.ToString(), *GetNameSafe(OwnerPiece));
		ShowSkillDebug(this, FString::Printf(TEXT("Input %s received (BP only, no native skill in slot)."), ToSlotString(FallbackSlot)), FColor::Silver, 1.5f);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[UCommandSkillComponent] Slot skill resolved. Slot=%s Action=%s Skill=%s"), ToSlotString(FallbackSlot), *InputActionName.ToString(), *RuntimeId.ToString());

	FString Reason;
	if (!CanExecuteSkillSpec(*SkillToExecute, Reason))
	{
		ShowSkillDebug(this, FString::Printf(TEXT("Blocked: %s (%s)"), *SkillToExecute->SkillId.ToString(), *Reason), FColor::Red, 2.5f);
		UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] Skill blocked. Id=%s Action=%s Reason=%s"), *SkillToExecute->SkillId.ToString(), *GetNameSafe(InputAction), *Reason);
		return;
	}

	if (!IsValid(OwnerPiece))
	{
		return;
	}

	if (OwnerPiece->HasAuthority())
	{
		ServerUseSkillById_Implementation(RuntimeId);
	}
	else
	{
		ServerUseSkillById(RuntimeId);
	}

	ShowSkillDebug(this, FString::Printf(TEXT("Cast request: %s (Slot=%s Action=%s)"), *SkillToExecute->SkillId.ToString(), ToSlotString(FallbackSlot), *GetNameSafe(InputAction)), FColor::Green, 2.0f);
}

void UCommandSkillComponent::ValidateSkillSpecs() const
{
	TSet<FName> SkillIds;
	TSet<ECommandSkillSlot> UsedSlots;
	TSet<FName> UsedTags;

	for (const FCommandSkillSpec& SkillSpec : Skills)
	{
		if (!SkillSpec.bEnabled)
		{
			continue;
		}

		if (SkillSpec.SkillType == ECommandSkillType::None)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] ValidateSkillSpecs: SkillType None. SkillId=%s"), *SkillSpec.SkillId.ToString());
		}

		if (SkillSpec.SkillId.IsNone())
		{
			UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] ValidateSkillSpecs: SkillId is None. DisplayName=%s"), *SkillSpec.DisplayName.ToString());
		}
		else if (SkillIds.Contains(SkillSpec.SkillId))
		{
			UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] ValidateSkillSpecs: duplicated SkillId=%s"), *SkillSpec.SkillId.ToString());
		}
		else
		{
			SkillIds.Add(SkillSpec.SkillId);
		}

		if (SkillSpec.InputSlot != ECommandSkillSlot::None)
		{
			if (UsedSlots.Contains(SkillSpec.InputSlot))
			{
				UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] ValidateSkillSpecs: duplicated Slot=%s"), ToSlotString(SkillSpec.InputSlot));
			}
			else
			{
				UsedSlots.Add(SkillSpec.InputSlot);
			}
		}

		if (!SkillSpec.InputTag.IsNone())
		{
			if (UsedTags.Contains(SkillSpec.InputTag))
			{
				UE_LOG(LogTemp, Warning, TEXT("[UCommandSkillComponent] ValidateSkillSpecs: duplicated InputTag=%s"), *SkillSpec.InputTag.ToString());
			}
			else
			{
				UsedTags.Add(SkillSpec.InputTag);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[UCommandSkillComponent] Skill Ready: Id=%s Name=%s Type=%s Slot=%s Tag=%s Magnitude=%.1f Secondary=%.1f Cooldown=%.2f Enabled=%s"),
			*SkillSpec.SkillId.ToString(),
			*SkillSpec.DisplayName.ToString(),
			ToSkillTypeString(SkillSpec.SkillType),
			ToSlotString(SkillSpec.InputSlot),
			*SkillSpec.InputTag.ToString(),
			SkillSpec.Magnitude,
			SkillSpec.SecondaryMagnitude,
			SkillSpec.Cooldown,
			SkillSpec.bEnabled ? TEXT("true") : TEXT("false"));
	}
}
