// Copyright Epic Games, Inc. All Rights Reserved.

#include "yjh_base/yjh_ArenaCombatantBase.h"

#include "Engine/OverlapResult.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "NSH/AuthoritativeJanggiBoard.h"
#include "Components/InputComponent.h"
#include "Engine/LocalPlayer.h"
#include "yjh_base/yjh_ArenaCombatComponent.h"
#include "yjh_base/yjh_ArenaHealthComponent.h"

AYJHArenaCombatantBase::AYJHArenaCombatantBase()
{
	bReplicates = true;
	SetReplicateMovement(true);

	HealthComponent = CreateDefaultSubobject<UYJHArenaHealthComponent>(TEXT("YJH_HealthComponent"));
	CombatComponent = CreateDefaultSubobject<UYJHArenaCombatComponent>(TEXT("YJH_CombatComponent"));
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
	if (!CanUsePrimaryAction())
	{
		return;
	}

	if (HasAuthority())
	{
		ExecutePrimaryActionServer();
		return;
	}

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

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}

	if (HealthComponent)
	{
		HealthComponent->OnDamaged.AddDynamic(this, &AYJHArenaCombatantBase::HandleDamaged);
		HealthComponent->OnDead.AddDynamic(this, &AYJHArenaCombatantBase::HandleDead);
	}
}

void AYJHArenaCombatantBase::BeginCombatSession(FName InCombatSessionId)
{
	CombatSessionId = InCombatSessionId;
	bCombatEnabled = true;
	bMovementEnabled = true;
	NextPrimaryActionTimeSeconds = 0.0;

	if (HealthComponent && HasAuthority())
	{
		HealthComponent->InitializeHealth(MaxHP);
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
	if (!CombatComponent || !bCombatEnabled)
	{
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
		return CombatComponent->RequestSkillBySlot(SlotIndex, RuntimeContext);
	}

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
	if (!CombatComponent || !bCombatEnabled)
	{
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

	CombatComponent->RequestSkillBySlot(SlotIndex, RuntimeContext);
}

void AYJHArenaCombatantBase::HandleDamaged(FName InCombatSessionId, FName InstigatorCombatantId, float Amount)
{
	OnDamagedVisualOnly(Amount);
}

void AYJHArenaCombatantBase::HandleDead(FName InCombatSessionId, FName DeadCombatantId, FName KillerCombatantId, FName EndReason)
{
	bCombatEnabled = false;
	bMovementEnabled = false;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
	}

	if (HasAuthority())
	{
		if (AAuthoritativeJanggiBoard* Board = Cast<AAuthoritativeJanggiBoard>(GetOwner()))
		{
			Board->ReportArenaCombatantDefeated(this);
		}
	}
	OnDeadVisualOnly();
}
