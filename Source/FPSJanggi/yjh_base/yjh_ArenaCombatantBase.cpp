// Copyright Epic Games, Inc. All Rights Reserved.

#include "yjh_base/yjh_ArenaCombatantBase.h"

#include "Camera/CameraComponent.h"
#include "Engine/OverlapResult.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Logging/LogMacros.h"
#include "Net/UnrealNetwork.h"
#include "NSH/AuthoritativeJanggiBoard.h"
#include "Components/InputComponent.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "yjh_base/yjh_ArenaCombatComponent.h"
#include "yjh_base/yjh_ArenaHealthComponent.h"
#include "yjh_base/yjh_SkillDataAsset.h"

DEFINE_LOG_CATEGORY_STATIC(LogYJHCombatant, Log, All);

namespace
{
	UYJHSkillDataAsset* TryLoadSkillDataAsset(const TCHAR* AssetPath)
	{
		return Cast<UYJHSkillDataAsset>(StaticLoadObject(UYJHSkillDataAsset::StaticClass(), nullptr, AssetPath));
	}

	UYJHSkillDataAsset* ResolveFallbackSkillDataAsset(const FName PieceInstanceId, const FName CombatantId)
	{
		TArray<FString> CandidateNames;
		if (!PieceInstanceId.IsNone())
		{
			CandidateNames.Add(PieceInstanceId.ToString());
		}
		if (!CombatantId.IsNone() && CombatantId != PieceInstanceId)
		{
			CandidateNames.Add(CombatantId.ToString());
		}

		for (const FString& Name : CandidateNames)
		{
			const FString Path = FString::Printf(TEXT("/Game/User/piece/%s/%s_skill.%s_skill"), *Name, *Name, *Name);
			if (UYJHSkillDataAsset* Loaded = TryLoadSkillDataAsset(*Path))
			{
				return Loaded;
			}
		}

		return nullptr;
	}

	template <typename TComponent>
	TComponent* EnsureArenaComponent(AActor* Owner, TObjectPtr<TComponent>& ComponentPtr, const TCHAR* RuntimeName)
	{
		if (!Owner)
		{
			return nullptr;
		}

		if (ComponentPtr)
		{
			return ComponentPtr.Get();
		}

		if (TComponent* ExistingComponent = Owner->FindComponentByClass<TComponent>())
		{
			ComponentPtr = ExistingComponent;
			return ExistingComponent;
		}

		TComponent* CreatedComponent = NewObject<TComponent>(Owner, FName(RuntimeName));
		if (!CreatedComponent)
		{
			return nullptr;
		}

		Owner->AddInstanceComponent(CreatedComponent);
		CreatedComponent->RegisterComponent();
		ComponentPtr = CreatedComponent;
		UE_LOG(LogYJHCombatant, Warning, TEXT("YJH_RUNTIME_COMPONENT_RESTORED owner=%s component=%s"),
			*GetNameSafe(Owner), *CreatedComponent->GetClass()->GetName());
		return CreatedComponent;
	}
}

AYJHArenaCombatantBase::AYJHArenaCombatantBase()
{
	bReplicates = true;
	SetReplicateMovement(true);

	HealthComponent = CreateDefaultSubobject<UYJHArenaHealthComponent>(TEXT("YJH_HealthComponent"));
	CombatComponent = CreateDefaultSubobject<UYJHArenaCombatComponent>(TEXT("YJH_CombatComponent"));
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("YJH_FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FirstPersonCameraOffset);
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->FieldOfView = FirstPersonFieldOfView;

	FirstPersonArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("YJH_FirstPersonArms"));
	FirstPersonArmsMesh->SetupAttachment(FirstPersonCameraComponent);
	FirstPersonArmsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonArmsMesh->SetCastShadow(false);
	FirstPersonArmsMesh->SetOnlyOwnerSee(true);
	FirstPersonArmsMesh->bCastDynamicShadow = false;
	FirstPersonArmsMesh->bReceivesDecals = false;
}

void AYJHArenaCombatantBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AYJHArenaCombatantBase, VisualScale);
	DOREPLIFETIME(AYJHArenaCombatantBase, HitboxScale);
}

void AYJHArenaCombatantBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent)
	{
		return;
	}

	if (TryBindEnhancedInput(PlayerInputComponent))
	{
		return;
	}

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AYJHArenaCombatantBase::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AYJHArenaCombatantBase::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AYJHArenaCombatantBase::TurnYaw);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AYJHArenaCombatantBase::LookPitch);
	PlayerInputComponent->BindAction(TEXT("PrimaryAction"), IE_Pressed, this, &AYJHArenaCombatantBase::PrimaryAction);
	PlayerInputComponent->BindAction(TEXT("SkillSlot1"), IE_Pressed, this, &AYJHArenaCombatantBase::TriggerSkillSlot1);
	PlayerInputComponent->BindAction(TEXT("SkillSlot2"), IE_Pressed, this, &AYJHArenaCombatantBase::TriggerSkillSlot2);
	PlayerInputComponent->BindAction(TEXT("SkillSlot3"), IE_Pressed, this, &AYJHArenaCombatantBase::TriggerSkillSlot3);
	PlayerInputComponent->BindAction(TEXT("SkillSlot4"), IE_Pressed, this, &AYJHArenaCombatantBase::TriggerSkillSlot4);
	PlayerInputComponent->BindAction(TEXT("SkillSlot5"), IE_Pressed, this, &AYJHArenaCombatantBase::TriggerSkillSlot5);
	PlayerInputComponent->BindAction(TEXT("SkillSlot6"), IE_Pressed, this, &AYJHArenaCombatantBase::TriggerSkillSlot6);
	PlayerInputComponent->BindAction(TEXT("SkillSlot7"), IE_Pressed, this, &AYJHArenaCombatantBase::TriggerSkillSlot7);
	PlayerInputComponent->BindAction(TEXT("SkillSlot8"), IE_Pressed, this, &AYJHArenaCombatantBase::TriggerSkillSlot8);
	PlayerInputComponent->BindAction(TEXT("SkillSlot9"), IE_Pressed, this, &AYJHArenaCombatantBase::TriggerSkillSlot9);
	PlayerInputComponent->BindAction(TEXT("SkillSlot10"), IE_Pressed, this, &AYJHArenaCombatantBase::TriggerSkillSlot10);
}

bool AYJHArenaCombatantBase::TryBindEnhancedInput(UInputComponent* PlayerInputComponent)
{
	if (!bUseEnhancedInputBindings)
	{
		return false;
	}

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		return false;
	}

	const bool bHasRequiredEnhancedActions = (IA_Move || IA_Look || IA_PrimaryAction || IA_SkillSlot1 || IA_SkillSlot2 || IA_SkillSlot3 || IA_SkillSlot4 || IA_SkillSlot5 || IA_SkillSlot6 || IA_SkillSlot7 || IA_SkillSlot8 || IA_SkillSlot9 || IA_SkillSlot10);
	if (!bHasRequiredEnhancedActions)
	{
		return false;
	}

	AddEnhancedInputMappingContexts();

	if (IA_Move)
	{
		EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AYJHArenaCombatantBase::HandleEnhancedMove);
	}
	if (IA_Look)
	{
		EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AYJHArenaCombatantBase::HandleEnhancedLook);
	}
	if (IA_PrimaryAction)
	{
		EnhancedInput->BindAction(IA_PrimaryAction, ETriggerEvent::Started, this, &AYJHArenaCombatantBase::HandleEnhancedPrimaryAction);
	}
	if (IA_SkillSlot1)
	{
		EnhancedInput->BindAction(IA_SkillSlot1, ETriggerEvent::Started, this, &AYJHArenaCombatantBase::HandleEnhancedSkillSlot1);
	}
	if (IA_SkillSlot2)
	{
		EnhancedInput->BindAction(IA_SkillSlot2, ETriggerEvent::Started, this, &AYJHArenaCombatantBase::HandleEnhancedSkillSlot2);
	}
	if (IA_SkillSlot3)
	{
		EnhancedInput->BindAction(IA_SkillSlot3, ETriggerEvent::Started, this, &AYJHArenaCombatantBase::HandleEnhancedSkillSlot3);
	}
	if (IA_SkillSlot4)
	{
		EnhancedInput->BindAction(IA_SkillSlot4, ETriggerEvent::Started, this, &AYJHArenaCombatantBase::HandleEnhancedSkillSlot4);
	}
	if (IA_SkillSlot5)
	{
		EnhancedInput->BindAction(IA_SkillSlot5, ETriggerEvent::Started, this, &AYJHArenaCombatantBase::HandleEnhancedSkillSlot5);
	}
	if (IA_SkillSlot6)
	{
		EnhancedInput->BindAction(IA_SkillSlot6, ETriggerEvent::Started, this, &AYJHArenaCombatantBase::HandleEnhancedSkillSlot6);
	}
	if (IA_SkillSlot7)
	{
		EnhancedInput->BindAction(IA_SkillSlot7, ETriggerEvent::Started, this, &AYJHArenaCombatantBase::HandleEnhancedSkillSlot7);
	}
	if (IA_SkillSlot8)
	{
		EnhancedInput->BindAction(IA_SkillSlot8, ETriggerEvent::Started, this, &AYJHArenaCombatantBase::HandleEnhancedSkillSlot8);
	}
	if (IA_SkillSlot9)
	{
		EnhancedInput->BindAction(IA_SkillSlot9, ETriggerEvent::Started, this, &AYJHArenaCombatantBase::HandleEnhancedSkillSlot9);
	}
	if (IA_SkillSlot10)
	{
		EnhancedInput->BindAction(IA_SkillSlot10, ETriggerEvent::Started, this, &AYJHArenaCombatantBase::HandleEnhancedSkillSlot10);
	}

	return true;
}

void AYJHArenaCombatantBase::AddEnhancedInputMappingContexts() const
{
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem)
	{
		return;
	}

	if (CommonInputMappingContext)
	{
		Subsystem->AddMappingContext(CommonInputMappingContext, CommonInputContextPriority);
	}

	if (CombatInputMappingContext)
	{
		Subsystem->AddMappingContext(CombatInputMappingContext, CombatInputContextPriority);
	}
}

void AYJHArenaCombatantBase::HandleEnhancedMove(const FInputActionValue& Value)
{
	const FVector2D MoveAxis = Value.Get<FVector2D>();
	MoveForward(MoveAxis.Y);
	MoveRight(MoveAxis.X);
}

void AYJHArenaCombatantBase::HandleEnhancedLook(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	TurnYaw(LookAxis.X);
	LookPitch(LookAxis.Y);
}

void AYJHArenaCombatantBase::HandleEnhancedPrimaryAction(const FInputActionValue& Value)
{
	PrimaryAction();
}

void AYJHArenaCombatantBase::HandleEnhancedSkillSlot1(const FInputActionValue& Value) { TriggerSkillSlot1(); }
void AYJHArenaCombatantBase::HandleEnhancedSkillSlot2(const FInputActionValue& Value) { TriggerSkillSlot2(); }
void AYJHArenaCombatantBase::HandleEnhancedSkillSlot3(const FInputActionValue& Value) { TriggerSkillSlot3(); }
void AYJHArenaCombatantBase::HandleEnhancedSkillSlot4(const FInputActionValue& Value) { TriggerSkillSlot4(); }
void AYJHArenaCombatantBase::HandleEnhancedSkillSlot5(const FInputActionValue& Value) { TriggerSkillSlot5(); }
void AYJHArenaCombatantBase::HandleEnhancedSkillSlot6(const FInputActionValue& Value) { TriggerSkillSlot6(); }
void AYJHArenaCombatantBase::HandleEnhancedSkillSlot7(const FInputActionValue& Value) { TriggerSkillSlot7(); }
void AYJHArenaCombatantBase::HandleEnhancedSkillSlot8(const FInputActionValue& Value) { TriggerSkillSlot8(); }
void AYJHArenaCombatantBase::HandleEnhancedSkillSlot9(const FInputActionValue& Value) { TriggerSkillSlot9(); }
void AYJHArenaCombatantBase::HandleEnhancedSkillSlot10(const FInputActionValue& Value) { TriggerSkillSlot10(); }

void AYJHArenaCombatantBase::MoveForward(float Value)
{
	if (!bMovementEnabled || FMath::IsNearlyZero(Value) || !Controller || (HealthComponent && HealthComponent->IsDead()))
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, Value);
}

void AYJHArenaCombatantBase::MoveRight(float Value)
{
	if (!bMovementEnabled || FMath::IsNearlyZero(Value) || !Controller || (HealthComponent && HealthComponent->IsDead()))
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Direction, Value);
}

void AYJHArenaCombatantBase::TurnYaw(float Value)
{
	if (!bMovementEnabled || FMath::IsNearlyZero(Value))
	{
		return;
	}

	AddControllerYawInput(Value * TurnRateScale);
}

void AYJHArenaCombatantBase::LookPitch(float Value)
{
	if (!bMovementEnabled || FMath::IsNearlyZero(Value))
	{
		return;
	}

	AddControllerPitchInput(Value * LookRateScale);
}

void AYJHArenaCombatantBase::SetWalkSpeed(float NewWalkSpeed)
{
	BaseWalkSpeed = FMath::Max(0.0f, NewWalkSpeed);
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}

void AYJHArenaCombatantBase::SetScaleStats(float InVisualScale, float InHitboxScale)
{
	VisualScale = FMath::Clamp(InVisualScale, 0.2f, 3.0f);
	HitboxScale = FMath::Clamp(InHitboxScale, 0.2f, 3.0f);
	ApplyScaleStats();
}

void AYJHArenaCombatantBase::ApplyScaleStats()
{
	CacheScaleDefaults();

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetRelativeScale3D(DefaultMeshScale * VisualScale);
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCapsuleSize(DefaultCapsuleRadius * HitboxScale, DefaultCapsuleHalfHeight * HitboxScale, true);
	}
}

void AYJHArenaCombatantBase::SetMovementEnabled(bool bEnabled)
{
	bMovementEnabled = bEnabled;
	if (!bMovementEnabled && GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
	}
}

void AYJHArenaCombatantBase::PrimaryAction()
{
	if (HasAuthority())
	{
		if (!CanUsePrimaryAction())
		{
			return;
		}

		ExecutePrimaryActionServer();
		return;
	}

	// Client-side combat/session flags can be briefly stale; server validates authoritative state.
	ServerPrimaryAction();
}

FYJHSkillExecutionResult AYJHArenaCombatantBase::TriggerSkillSlotByIndex(int32 SlotNumber)
{
	const FName SlotName = BuildSlotName(SlotNumber);
	FYJHRuntimeSkillRequestContext RuntimeContext;
	return TriggerSkillBySlot(SlotName, RuntimeContext);
}

void AYJHArenaCombatantBase::ServerPrimaryAction_Implementation()
{
	if (!CanUsePrimaryAction())
	{
		return;
	}

	ExecutePrimaryActionServer();
}

void AYJHArenaCombatantBase::ExecutePrimaryActionServer()
{
	if (!GetWorld() || !HealthComponent || HealthComponent->IsDead())
	{
		return;
	}

	NextPrimaryActionTimeSeconds = GetWorld()->GetTimeSeconds() + BasicAttackCooldownSeconds;

	const FVector Start = GetActorLocation() + GetActorForwardVector() * FMath::Max(0.0f, BasicAttackRange * 0.5f);
	const FCollisionShape Shape = FCollisionShape::MakeSphere(FMath::Max(1.0f, BasicAttackRadius));
	FCollisionQueryParams Params(SCENE_QUERY_STAT(YJH_PrimaryAttack), false, this);

	TArray<FOverlapResult> Overlaps;
	if (!GetWorld()->OverlapMultiByChannel(Overlaps, Start, FQuat::Identity, ECC_Pawn, Shape, Params))
	{
		return;
	}

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* TargetActor = Overlap.GetActor();
		if (!TargetActor || TargetActor == this)
		{
			continue;
		}

		AYJHArenaCombatantBase* TargetCombatant = Cast<AYJHArenaCombatantBase>(TargetActor);
		if (!TargetCombatant || TargetCombatant->TeamInfo == TeamInfo || !TargetCombatant->HealthComponent || TargetCombatant->HealthComponent->IsDead())
		{
			continue;
		}

		TargetCombatant->HealthComponent->ApplyServerDamage(CombatSessionId, CombatantId, BasicAttackDamage);
		break;
	}
}

bool AYJHArenaCombatantBase::CanUsePrimaryAction() const
{
	if (!bCombatEnabled || !bMovementEnabled || !GetWorld() || !HealthComponent || HealthComponent->IsDead())
	{
		return false;
	}

	return GetWorld()->GetTimeSeconds() >= NextPrimaryActionTimeSeconds;
}

void AYJHArenaCombatantBase::BeginPlay()
{
	Super::BeginPlay();
	EnsureArenaComponent(this, HealthComponent, TEXT("YJH_HealthComponent_Runtime"));
	EnsureArenaComponent(this, CombatComponent, TEXT("YJH_CombatComponent_Runtime"));
	if (CombatComponent && SkillDataAssetOverride)
	{
		CombatComponent->SkillDataAsset = SkillDataAssetOverride;
	}
	if (CombatComponent && !CombatComponent->SkillDataAsset)
	{
		CombatComponent->SkillDataAsset = ResolveFallbackSkillDataAsset(PieceInstanceId, CombatantId);
		if (!CombatComponent->SkillDataAsset)
		{
			UE_LOG(LogYJHCombatant, Warning, TEXT("YJH_SKILLDATA_UNRESOLVED pawn=%s piece=%s combatant=%s"),
				*GetNameSafe(this), *PieceInstanceId.ToString(), *CombatantId.ToString());
		}
	}
	if (FirstPersonCameraComponent)
	{
		FirstPersonCameraComponent->SetRelativeLocation(FirstPersonCameraOffset);
		FirstPersonCameraComponent->SetFieldOfView(FirstPersonFieldOfView);
	}
	RefreshFirstPersonPresentation();
	CacheScaleDefaults();
	ApplyScaleStats();

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}

	if (HealthComponent)
	{
		HealthComponent->OnDamaged.AddUniqueDynamic(this, &AYJHArenaCombatantBase::HandleDamaged);
		HealthComponent->OnDead.AddUniqueDynamic(this, &AYJHArenaCombatantBase::HandleDead);
	}
}

void AYJHArenaCombatantBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	RefreshFirstPersonPresentation();
}

void AYJHArenaCombatantBase::OnRep_Controller()
{
	Super::OnRep_Controller();
	RefreshFirstPersonPresentation();
}

void AYJHArenaCombatantBase::BeginCombatSession(FName InCombatSessionId)
{
	EnsureArenaComponent(this, HealthComponent, TEXT("YJH_HealthComponent_Runtime"));
	EnsureArenaComponent(this, CombatComponent, TEXT("YJH_CombatComponent_Runtime"));
	if (CombatComponent && SkillDataAssetOverride)
	{
		CombatComponent->SkillDataAsset = SkillDataAssetOverride;
	}
	if (CombatComponent && !CombatComponent->SkillDataAsset)
	{
		CombatComponent->SkillDataAsset = ResolveFallbackSkillDataAsset(PieceInstanceId, CombatantId);
		if (!CombatComponent->SkillDataAsset)
		{
			UE_LOG(LogYJHCombatant, Warning, TEXT("YJH_SKILLDATA_UNRESOLVED pawn=%s piece=%s combatant=%s"),
				*GetNameSafe(this), *PieceInstanceId.ToString(), *CombatantId.ToString());
		}
	}

	CombatSessionId = InCombatSessionId;
	bDeathHandledForSession = false;
	bCombatEnabled = true;
	bMovementEnabled = true;
	NextPrimaryActionTimeSeconds = 0.0;

	if (HealthComponent && HasAuthority())
	{
		HealthComponent->InitializeHealth(MaxHP);
	}

	if (HealthComponent)
	{
		HealthComponent->OnDamaged.RemoveDynamic(this, &AYJHArenaCombatantBase::HandleDamaged);
		HealthComponent->OnDead.RemoveDynamic(this, &AYJHArenaCombatantBase::HandleDead);
		HealthComponent->OnDamaged.AddUniqueDynamic(this, &AYJHArenaCombatantBase::HandleDamaged);
		HealthComponent->OnDead.AddUniqueDynamic(this, &AYJHArenaCombatantBase::HandleDead);
	}

	if (CombatComponent)
	{
		CombatComponent->SetCombatContext(CombatSessionId, CombatantId);
		CombatComponent->InitializeSkillMap();
	}
}

FName AYJHArenaCombatantBase::BuildSlotName(int32 SlotNumber) const
{
	const int32 Clamped = FMath::Clamp(SlotNumber, 1, 10);
	return FName(*FString::Printf(TEXT("Slot%d"), Clamped));
}

void AYJHArenaCombatantBase::TriggerSkillSlot1() { TriggerSkillSlotByIndex(1); }
void AYJHArenaCombatantBase::TriggerSkillSlot2() { TriggerSkillSlotByIndex(2); }
void AYJHArenaCombatantBase::TriggerSkillSlot3() { TriggerSkillSlotByIndex(3); }
void AYJHArenaCombatantBase::TriggerSkillSlot4() { TriggerSkillSlotByIndex(4); }
void AYJHArenaCombatantBase::TriggerSkillSlot5() { TriggerSkillSlotByIndex(5); }
void AYJHArenaCombatantBase::TriggerSkillSlot6() { TriggerSkillSlotByIndex(6); }
void AYJHArenaCombatantBase::TriggerSkillSlot7() { TriggerSkillSlotByIndex(7); }
void AYJHArenaCombatantBase::TriggerSkillSlot8() { TriggerSkillSlotByIndex(8); }
void AYJHArenaCombatantBase::TriggerSkillSlot9() { TriggerSkillSlotByIndex(9); }
void AYJHArenaCombatantBase::TriggerSkillSlot10() { TriggerSkillSlotByIndex(10); }

void AYJHArenaCombatantBase::EndCombatSession(FName InCombatSessionId, FName EndReason)
{
	if (CombatSessionId != InCombatSessionId)
	{
		return;
	}

	bCombatEnabled = false;
	bMovementEnabled = false;
	CombatSessionId = NAME_None;
	OnDeadVisualOnly();
}

void AYJHArenaCombatantBase::SetCombatEnabled(FName InCombatSessionId, bool bEnabled)
{
	if (CombatSessionId != InCombatSessionId)
	{
		return;
	}
	bCombatEnabled = bEnabled;
	bMovementEnabled = bEnabled;
}

FYJHSkillExecutionResult AYJHArenaCombatantBase::TriggerSkillBySlot(FName SlotIndex, FYJHRuntimeSkillRequestContext RuntimeContext)
{
	EnsureArenaComponent(this, HealthComponent, TEXT("YJH_HealthComponent_Runtime"));
	EnsureArenaComponent(this, CombatComponent, TEXT("YJH_CombatComponent_Runtime"));

	if (HasAuthority() && !bCombatEnabled)
	{
		UE_LOG(LogYJHCombatant, Warning, TEXT("YJH_SKILL_TRIGGER_BLOCKED pawn=%s slot=%s has_component=%s combat_enabled=%s session=%s"),
			*GetNameSafe(this), *SlotIndex.ToString(), CombatComponent ? TEXT("true") : TEXT("false"), bCombatEnabled ? TEXT("true") : TEXT("false"), *CombatSessionId.ToString());
		FYJHSkillExecutionResult Result;
		Result.bSuccess = false;
		Result.SkillId = NAME_None;
		Result.SlotIndex = SlotIndex;
		Result.FailCode = EYJHSkillFailCode::SKX_ExecutionBlocked;
		return Result;
	}

	if (HasAuthority() && !CombatComponent)
	{
		UE_LOG(LogYJHCombatant, Warning, TEXT("YJH_SKILL_TRIGGER_BLOCKED pawn=%s slot=%s has_component=false combat_enabled=true session=%s reason=NoCombatComponentOnServer"),
			*GetNameSafe(this), *SlotIndex.ToString(), *CombatSessionId.ToString());
		FYJHSkillExecutionResult Result;
		Result.bSuccess = false;
		Result.SkillId = NAME_None;
		Result.SlotIndex = SlotIndex;
		Result.FailCode = EYJHSkillFailCode::SKX_ExecutionBlocked;
		return Result;
	}

	RuntimeContext.CombatSessionId = CombatSessionId;
	RuntimeContext.CombatantId = CombatantId;
	RuntimeContext.SlotIndex = SlotIndex;
	RuntimeContext.SourceLocation = GetActorLocation();
	RuntimeContext.SourceForward = GetActorForwardVector();
	if (RuntimeContext.RequestedDirection.IsNearlyZero())
	{
		RuntimeContext.RequestedDirection = GetActorForwardVector();
	}

	if (HasAuthority())
	{
		const FYJHSkillExecutionResult Result = CombatComponent->RequestSkillBySlot(SlotIndex, RuntimeContext);
		UE_LOG(LogYJHCombatant, Display, TEXT("YJH_SKILL_TRIGGER_SERVER pawn=%s slot=%s success=%s fail=%s session=%s"),
			*GetNameSafe(this), *SlotIndex.ToString(), Result.bSuccess ? TEXT("true") : TEXT("false"), *UEnum::GetValueAsString(Result.FailCode), *CombatSessionId.ToString());
		return Result;
	}

	if (!bCombatEnabled || CombatSessionId.IsNone())
	{
		UE_LOG(LogYJHCombatant, Warning, TEXT("YJH_SKILL_CLIENT_STATE_STALE pawn=%s slot=%s combat_enabled=%s session=%s -- sending RPC for server-side validation"),
			*GetNameSafe(this), *SlotIndex.ToString(), bCombatEnabled ? TEXT("true") : TEXT("false"), *CombatSessionId.ToString());
	}

	UE_LOG(LogYJHCombatant, Display, TEXT("YJH_SKILL_TRIGGER_CLIENT pawn=%s slot=%s session=%s"),
		*GetNameSafe(this), *SlotIndex.ToString(), *CombatSessionId.ToString());
	ServerTriggerSkillBySlot(SlotIndex, RuntimeContext);

	FYJHSkillExecutionResult PendingResult;
	PendingResult.bSuccess = true;
	PendingResult.SkillId = RuntimeContext.SkillId;
	PendingResult.SlotIndex = SlotIndex;
	PendingResult.FailCode = EYJHSkillFailCode::None;
	return PendingResult;
}

void AYJHArenaCombatantBase::ServerTriggerSkillBySlot_Implementation(FName SlotIndex, FYJHRuntimeSkillRequestContext RuntimeContext)
{
	EnsureArenaComponent(this, HealthComponent, TEXT("YJH_HealthComponent_Runtime"));
	EnsureArenaComponent(this, CombatComponent, TEXT("YJH_CombatComponent_Runtime"));

	if (!CombatComponent || !bCombatEnabled)
	{
		UE_LOG(LogYJHCombatant, Warning, TEXT("YJH_SKILL_TRIGGER_SERVER_BLOCKED pawn=%s slot=%s has_component=%s combat_enabled=%s session=%s"),
			*GetNameSafe(this), *SlotIndex.ToString(), CombatComponent ? TEXT("true") : TEXT("false"), bCombatEnabled ? TEXT("true") : TEXT("false"), *CombatSessionId.ToString());
		return;
	}

	RuntimeContext.CombatSessionId = CombatSessionId;
	RuntimeContext.CombatantId = CombatantId;
	RuntimeContext.SlotIndex = SlotIndex;
	RuntimeContext.SourceLocation = GetActorLocation();
	RuntimeContext.SourceForward = GetActorForwardVector();
	if (RuntimeContext.RequestedDirection.IsNearlyZero())
	{
		RuntimeContext.RequestedDirection = GetActorForwardVector();
	}

	const FYJHSkillExecutionResult Result = CombatComponent->RequestSkillBySlot(SlotIndex, RuntimeContext);
	UE_LOG(LogYJHCombatant, Display, TEXT("YJH_SKILL_TRIGGER_SERVER_RPC pawn=%s slot=%s success=%s fail=%s session=%s"),
		*GetNameSafe(this), *SlotIndex.ToString(), Result.bSuccess ? TEXT("true") : TEXT("false"), *UEnum::GetValueAsString(Result.FailCode), *CombatSessionId.ToString());

	TArray<float> CooldownSnapshot;
	const int32 SnapshotSlots = FMath::Clamp(CombatComponent->MaxSkillSlots, 1, 10);
	CooldownSnapshot.Reserve(SnapshotSlots);
	for (int32 SlotNumber = 1; SlotNumber <= SnapshotSlots; ++SlotNumber)
	{
		const FName SnapshotSlot(*FString::Printf(TEXT("Slot%d"), SlotNumber));
		CooldownSnapshot.Add(CombatComponent->GetRemainingCooldownBySlot(SnapshotSlot));
	}
	ClientSyncSkillCooldownSnapshot(CooldownSnapshot);
}

void AYJHArenaCombatantBase::ClientSyncSkillCooldownSnapshot_Implementation(const TArray<float>& SlotRemainingSeconds)
{
	EnsureArenaComponent(this, HealthComponent, TEXT("YJH_HealthComponent_Runtime"));
	EnsureArenaComponent(this, CombatComponent, TEXT("YJH_CombatComponent_Runtime"));
	if (CombatComponent)
	{
		CombatComponent->ApplyClientCooldownSnapshot(SlotRemainingSeconds);
	}
}

void AYJHArenaCombatantBase::HandleDamaged(FName InCombatSessionId, FName InstigatorCombatantId, float Amount)
{
	OnDamagedVisualOnly(Amount);

	// Fail-safe: if HP reached zero but a dead callback was missed, force the same path.
	if (HasAuthority() && HealthComponent && HealthComponent->IsDead() && !bDeathHandledForSession)
	{
		HandleDead(InCombatSessionId, CombatantId, InstigatorCombatantId, FName(TEXT("Eliminated")));
	}
}

void AYJHArenaCombatantBase::HandleDead(FName InCombatSessionId, FName DeadCombatantId, FName KillerCombatantId, FName EndReason)
{
	if (bDeathHandledForSession)
	{
		return;
	}
	bDeathHandledForSession = true;

	bCombatEnabled = false;
	bMovementEnabled = false;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
	}

	if (HasAuthority())
	{
		AAuthoritativeJanggiBoard* Board = Cast<AAuthoritativeJanggiBoard>(GetOwner());
		if (!Board)
		{
			for (TActorIterator<AAuthoritativeJanggiBoard> It(GetWorld()); It; ++It)
			{
				Board = *It;
				break;
			}
		}

		if (Board)
		{
			Board->ReportArenaCombatantDefeated(this);
		}
		else
		{
			UE_LOG(LogYJHCombatant, Warning, TEXT("YJH_DEATH_NO_BOARD pawn=%s owner=%s session=%s"),
				*GetNameSafe(this), *GetNameSafe(GetOwner()), *CombatSessionId.ToString());
		}
	}
	OnDeadVisualOnly();
}

void AYJHArenaCombatantBase::OnRep_VisualScale()
{
	ApplyScaleStats();
}

void AYJHArenaCombatantBase::OnRep_HitboxScale()
{
	ApplyScaleStats();
}

void AYJHArenaCombatantBase::CacheScaleDefaults()
{
	if (bScaleDefaultsCached)
	{
		return;
	}

	if (const UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		DefaultCapsuleRadius = Capsule->GetUnscaledCapsuleRadius();
		DefaultCapsuleHalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
	}
	else
	{
		DefaultCapsuleRadius = 42.0f;
		DefaultCapsuleHalfHeight = 96.0f;
	}

	if (const USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		DefaultMeshScale = MeshComponent->GetRelativeScale3D();
	}

	bScaleDefaultsCached = true;
}

void AYJHArenaCombatantBase::RefreshFirstPersonPresentation()
{
	const bool bLocalOwner = IsLocallyControlled();
	const bool bUseFirstPerson = bEnableFirstPersonPresentation && bLocalOwner;

	if (FirstPersonCameraComponent)
	{
		FirstPersonCameraComponent->SetRelativeLocation(FirstPersonCameraOffset);
		FirstPersonCameraComponent->SetFieldOfView(FirstPersonFieldOfView);
		FirstPersonCameraComponent->SetActive(bUseFirstPerson);
	}

	if (USkeletalMeshComponent* ThirdPersonMesh = GetMesh())
	{
		ThirdPersonMesh->SetOwnerNoSee(bUseFirstPerson && bHideThirdPersonMeshForOwner);
	}

	if (FirstPersonArmsMesh)
	{
		FirstPersonArmsMesh->SetOnlyOwnerSee(true);
		FirstPersonArmsMesh->SetOwnerNoSee(false);
		FirstPersonArmsMesh->SetVisibility(bUseFirstPerson, true);
	}
}
