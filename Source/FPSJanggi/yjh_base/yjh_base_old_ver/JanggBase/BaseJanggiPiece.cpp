// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseJanggiPiece.h"
#include "ArenaGameMode.h"
#include "CommandSkillComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "InputCoreTypes.h"
#include "GameFramework/PlayerController.h"

ABaseJanggiPiece::ABaseJanggiPiece()
{
	PrimaryActorTick.bCanEverTick = true;

	// 멀티플레이어 Replication 활성화
	bReplicates = true;
	SetReplicateMovement(true);

	// 기본 스탯 초기화
	MaxHP = 100.0f;
	HPRegenPerSecond = 2.0f;
	bUseAutoRegen = false;
	RegenDelayAfterHit = 3.0f;
	CurrentHP = MaxHP;
	MaxWalkSpeed = 600.0f;
	MaxAcceleration = 2048.0f;
	BrakingDecelerationWalking = 2048.0f;
	GroundFriction = 8.0f;
	RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	AirControl = 0.2f;
	JumpZVelocity = 420.0f;
	BaseAttackDamage = 25.0f;
	AttackRange = 220.0f;
	HitRadius = 60.0f;
	AttackCooldown = 0.5f;
	HitStunDuration = 0.2f;
	KnockbackStrength = 600.0f;
	HitboxScale = 1.0f;
	DamageMultiplier = 1.0f;
	StunResistance = 0.0f;
	SlowResistance = 0.0f;
	KnockbackResistance = 0.0f;
	Faction = EJanggiFaction::None;
	CharacterState = EJanggiPieceState::Fighting;
	DefaultInputMappingContext = nullptr;
	MoveAction = nullptr;
	LookAction = nullptr;
	bInputFallbackInitialized = false;
	FallbackInputMappingContext = nullptr;
	FallbackMoveAction = nullptr;
	FallbackLookAction = nullptr;
	bHasReportedDeath = false;
	LastTimeDamaged = -1000.0f;
	BaseCapsuleRadius = 42.0f;
	BaseCapsuleHalfHeight = 96.0f;
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	CommandSkillComponent = CreateDefaultSubobject<UCommandSkillComponent>(TEXT("CommandSkillComponent"));

	// 1인칭 공통 카메라 구성 (자식 BP에서 추가 카메라 없이 즉시 사용)
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	if (IsValid(FirstPersonCamera))
	{
		FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
		FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight));
		FirstPersonCamera->bUsePawnControlRotation = true;
	}

	// 1인칭 전용 팔/손 메쉬 슬롯 (현재는 빈 컴포넌트로 두고 필요 시 자식 BP에서 자산 할당)
	FirstPersonArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArmsMesh"));
	if (IsValid(FirstPersonArmsMesh) && IsValid(FirstPersonCamera))
	{
		FirstPersonArmsMesh->SetupAttachment(FirstPersonCamera);
		FirstPersonArmsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FirstPersonArmsMesh->SetCastShadow(false);
		FirstPersonArmsMesh->SetOnlyOwnerSee(true);
		FirstPersonArmsMesh->bCastHiddenShadow = false;
	}

	// 이동 컴포넌트 기본 속도 설정 (nullptr 가드)
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (IsValid(MoveComp))
	{
		MoveComp->MaxWalkSpeed = MaxWalkSpeed;
		MoveComp->MaxAcceleration = MaxAcceleration;
		MoveComp->BrakingDecelerationWalking = BrakingDecelerationWalking;
		MoveComp->GroundFriction = GroundFriction;
		MoveComp->RotationRate = RotationRate;
		MoveComp->AirControl = AirControl;
		MoveComp->JumpZVelocity = JumpZVelocity;
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->bUseControllerDesiredRotation = true;
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (IsValid(CapsuleComp))
	{
		BaseCapsuleRadius = CapsuleComp->GetUnscaledCapsuleRadius();
		BaseCapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
	}

	// 기본 Character Mesh는 3인칭 바디 용도로 유지
	USkeletalMeshComponent* BodyMesh = GetMesh();
	if (IsValid(BodyMesh))
	{
		BodyMesh->SetOwnerNoSee(true);
	}
}

void ABaseJanggiPiece::BeginPlay()
{
	Super::BeginPlay();
	EnsureInputFallback();

	ApplyMovementStatsToMovementComponent();

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (IsValid(CapsuleComp))
	{
		const float Scale = FMath::Max(0.1f, HitboxScale);
		CapsuleComp->SetCapsuleSize(BaseCapsuleRadius * Scale, BaseCapsuleHalfHeight * Scale, true);
	}

	// 서버(Authority)인 경우, 초기 체력 세팅
	if (HasAuthority())
	{
		CurrentHP = MaxHP;
		bHasReportedDeath = false;
		LastTimeDamaged = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	}

	AddDefaultInputMapping();
}

void ABaseJanggiPiece::EnsureInputFallback()
{
	if (bInputFallbackInitialized)
	{
		return;
	}

	FallbackInputMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Fallback_MoveLook"));
	FallbackMoveAction = NewObject<UInputAction>(this, TEXT("IA_Fallback_Move"));
	FallbackLookAction = NewObject<UInputAction>(this, TEXT("IA_Fallback_Look"));

	if (!IsValid(FallbackInputMappingContext) || !IsValid(FallbackMoveAction) || !IsValid(FallbackLookAction))
	{
		return;
	}

	FallbackMoveAction->ValueType = EInputActionValueType::Axis2D;
	FallbackLookAction->ValueType = EInputActionValueType::Axis2D;

	auto AddMoveKey = [this](const FKey& Key, bool bSwizzleYXZ, bool bNegX, bool bNegY)
	{
		FEnhancedActionKeyMapping& Mapping = FallbackInputMappingContext->MapKey(FallbackMoveAction, Key);

		if (bSwizzleYXZ)
		{
			UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(FallbackInputMappingContext);
			if (IsValid(Swizzle))
			{
				Swizzle->Order = EInputAxisSwizzle::YXZ;
				Mapping.Modifiers.Add(Swizzle);
			}
		}

		if (bNegX || bNegY)
		{
			UInputModifierNegate* Negate = NewObject<UInputModifierNegate>(FallbackInputMappingContext);
			if (IsValid(Negate))
			{
				Negate->bX = bNegX;
				Negate->bY = bNegY;
				Mapping.Modifiers.Add(Negate);
			}
		}
	};

	// WASD -> Axis2D
	AddMoveKey(EKeys::W, true, false, false);   // +Y
	AddMoveKey(EKeys::S, true, false, true);    // -Y
	AddMoveKey(EKeys::A, false, true, false);   // -X
	AddMoveKey(EKeys::D, false, false, false);  // +X

	// MouseX/MouseY -> Axis2D Look
	FallbackInputMappingContext->MapKey(FallbackLookAction, EKeys::MouseX);
	FEnhancedActionKeyMapping& MouseY = FallbackInputMappingContext->MapKey(FallbackLookAction, EKeys::MouseY);
	UInputModifierSwizzleAxis* MouseYSwizzle = NewObject<UInputModifierSwizzleAxis>(FallbackInputMappingContext);
	if (IsValid(MouseYSwizzle))
	{
		MouseYSwizzle->Order = EInputAxisSwizzle::YXZ;
		MouseY.Modifiers.Add(MouseYSwizzle);
	}
	UInputModifierNegate* MouseYNegate = NewObject<UInputModifierNegate>(FallbackInputMappingContext);
	if (IsValid(MouseYNegate))
	{
		MouseYNegate->bY = true;
		MouseY.Modifiers.Add(MouseYNegate);
	}

	bInputFallbackInitialized = true;
}

void ABaseJanggiPiece::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AddDefaultInputMapping();
}

void ABaseJanggiPiece::OnRep_Controller()
{
	Super::OnRep_Controller();
	AddDefaultInputMapping();
}

void ABaseJanggiPiece::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 런타임 중요 변수 복제 등록
	DOREPLIFETIME(ABaseJanggiPiece, CurrentHP);
	DOREPLIFETIME(ABaseJanggiPiece, CharacterState);
}

void ABaseJanggiPiece::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleAutoRegen(DeltaTime);
}

void ABaseJanggiPiece::ApplyMovementStatsToMovementComponent()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!IsValid(MoveComp))
	{
		return;
	}

	MoveComp->MaxWalkSpeed = MaxWalkSpeed;
	MoveComp->MaxAcceleration = MaxAcceleration;
	MoveComp->BrakingDecelerationWalking = BrakingDecelerationWalking;
	MoveComp->GroundFriction = GroundFriction;
	MoveComp->RotationRate = RotationRate;
	MoveComp->AirControl = AirControl;
	MoveComp->JumpZVelocity = JumpZVelocity;
}

void ABaseJanggiPiece::HandleAutoRegen(float DeltaTime)
{
	if (!HasAuthority() || !bUseAutoRegen || HPRegenPerSecond <= 0.0f)
	{
		return;
	}

	if (CharacterState == EJanggiPieceState::Dead || CurrentHP >= MaxHP)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	if (World->GetTimeSeconds() - LastTimeDamaged < RegenDelayAfterHit)
	{
		return;
	}

	const float OldHP = CurrentHP;
	CurrentHP = FMath::Clamp(CurrentHP + (HPRegenPerSecond * DeltaTime), 0.0f, MaxHP);
	if (!FMath::IsNearlyEqual(CurrentHP, OldHP))
	{
		OnRep_CurrentHP(OldHP);
	}
}

void ABaseJanggiPiece::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	EnsureInputFallback();
	AddDefaultInputMapping();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!IsValid(EnhancedInputComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BaseJanggiPiece] SetupPlayerInputComponent failed: EnhancedInputComponent invalid on %s"), *GetName());
		return;
	}

	if (IsValid(MoveAction))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABaseJanggiPiece::Move);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[BaseJanggiPiece] MoveAction is not assigned on %s"), *GetName());
	}

	if (IsValid(LookAction))
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABaseJanggiPiece::Look);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[BaseJanggiPiece] LookAction is not assigned on %s"), *GetName());
	}

	if (IsValid(CommandSkillComponent))
	{
		CommandSkillComponent->BindInput(EnhancedInputComponent);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[BaseJanggiPiece] CommandSkillComponent is invalid on %s"), *GetName());
	}

	if (IsValid(FallbackMoveAction) && FallbackMoveAction != MoveAction)
	{
		EnhancedInputComponent->BindAction(FallbackMoveAction, ETriggerEvent::Triggered, this, &ABaseJanggiPiece::Move);
	}

	if (IsValid(FallbackLookAction) && FallbackLookAction != LookAction)
	{
		EnhancedInputComponent->BindAction(FallbackLookAction, ETriggerEvent::Triggered, this, &ABaseJanggiPiece::Look);
	}
}

void ABaseJanggiPiece::AddDefaultInputMapping()
{
	EnsureInputFallback();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController())
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!IsValid(Subsystem))
	{
		return;
	}

	if (!IsValid(DefaultInputMappingContext))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BaseJanggiPiece] DefaultInputMappingContext is not assigned on %s"), *GetName());
	}

	if (!IsValid(MoveAction))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BaseJanggiPiece] MoveAction is not assigned on %s"), *GetName());
	}

	if (!IsValid(LookAction))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BaseJanggiPiece] LookAction is not assigned on %s"), *GetName());
	}

	if (IsValid(DefaultInputMappingContext))
	{
		Subsystem->RemoveMappingContext(DefaultInputMappingContext);
		Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
		UE_LOG(LogTemp, Log, TEXT("[BaseJanggiPiece] Added DefaultInputMappingContext=%s on %s"), *GetNameSafe(DefaultInputMappingContext), *GetName());
	}

	if (IsValid(FallbackInputMappingContext) && FallbackInputMappingContext != DefaultInputMappingContext)
	{
		Subsystem->RemoveMappingContext(FallbackInputMappingContext);
		Subsystem->AddMappingContext(FallbackInputMappingContext, 1);
	}
}

void ABaseJanggiPiece::Move(const FInputActionValue& Value)
{
	const FVector2D InputVector = Value.Get<FVector2D>();

	if (!CanMove())
	{
		return;
	}

	if (FMath::IsNearlyZero(InputVector.X) && FMath::IsNearlyZero(InputVector.Y))
	{
		return;
	}

	const FRotator ControlRotation = GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (!FMath::IsNearlyZero(InputVector.Y))
	{
		AddMovementInput(ForwardDirection, InputVector.Y);
	}

	if (!FMath::IsNearlyZero(InputVector.X))
	{
		AddMovementInput(RightDirection, InputVector.X);
	}
}

void ABaseJanggiPiece::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();

	if (!CanLook())
	{
		return;
	}

	if (!FMath::IsNearlyZero(LookVector.X))
	{
		AddControllerYawInput(LookVector.X);
	}

	if (!FMath::IsNearlyZero(LookVector.Y))
	{
		AddControllerPitchInput(LookVector.Y);
	}
}

float ABaseJanggiPiece::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 1. 서버 Authority 및 데미지 유효성 검증
	if (!HasAuthority() || DamageAmount <= 0.0f || CharacterState == EJanggiPieceState::Dead)
	{
		return 0.0f;
	}

	// 2. 아군 사격(Friendly Fire) 방지 로직 (선택적 가드)
	if (IsValid(DamageCauser))
	{
		ABaseJanggiPiece* AttackerPiece = Cast<ABaseJanggiPiece>(DamageCauser);
		if (IsValid(AttackerPiece) && AttackerPiece != this && AttackerPiece->GetFaction() != EJanggiFaction::None && AttackerPiece->GetFaction() == this->GetFaction())
		{
			// 같은 진영이면 데미지 무시
			return 0.0f;
		}
	}

	const float AdjustedIncomingDamage = FMath::Max(0.0f, DamageAmount * DamageMultiplier);

	// 3. 상위 TakeDamage 호출 및 실제 체력 계산
	const float ActualDamage = Super::TakeDamage(AdjustedIncomingDamage, DamageEvent, EventInstigator, DamageCauser);
	const float OldHP = CurrentHP;
	
	CurrentHP = FMath::Clamp(CurrentHP - ActualDamage, 0.0f, MaxHP);
	LastTimeDamaged = GetWorld() ? GetWorld()->GetTimeSeconds() : LastTimeDamaged;

	// 4. 서버 측 UI 및 이펙트 이벤트를 위해 즉시 OnRep 및 BP 이벤트 호출 (서버는 OnRep이 자동 호출되지 않음)
	OnRep_CurrentHP(OldHP);
	OnTakeDamageEvent(ActualDamage, CurrentHP, DamageCauser);

	// 5. 사망 판정
	if (CurrentHP <= 0.0f)
	{
		HandleDeath(DamageCauser);
	}

	return ActualDamage;
}

void ABaseJanggiPiece::HandleDeath(AActor* Killer)
{
	if (!HasAuthority() || CharacterState == EJanggiPieceState::Dead)
	{
		return;
	}

	// 상태를 사망으로 변경 (서버에서는 즉시 OnRep_CharacterState 호출 필요)
	EJanggiPieceState OldState = CharacterState;
	CharacterState = EJanggiPieceState::Dead;
	
	OnRep_CharacterState(OldState);

	if (!bHasReportedDeath)
	{
		bHasReportedDeath = true;

		if (UWorld* World = GetWorld())
		{
			AArenaGameMode* ArenaGameMode = Cast<AArenaGameMode>(World->GetAuthGameMode());
			if (IsValid(ArenaGameMode))
			{
				ArenaGameMode->ReportPieceDeath(this, Killer);
			}
			else
			{
				UE_LOG(LogTemp, Verbose, TEXT("[ABaseJanggiPiece] Death report skipped. AuthGameMode is not AArenaGameMode. Piece=%s"), *GetNameSafe(this));
			}
		}
	}

	// 사망 네이티브 이벤트 호출 (C++ 로직 후 BP 로직 수행)
	OnDeath(Killer);
}

void ABaseJanggiPiece::OnDeath_Implementation(AActor* Killer)
{
	// [C++ 기본 사망 처리]
	
	// 1. 캡슐 콜리전 비활성화로 다른 플레이어가 지나갈 수 있게 처리
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (IsValid(CapsuleComp))
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	// 2. 캐릭터 이동 및 입력 완전 중지
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (IsValid(MoveComp))
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
		MoveComp->SetComponentTickEnabled(false);
	}

	// 3. 컨트롤러 안전 해제 (서버에서 빙의를 해제하여 레벨 트래블 및 참조 문제 방지)
	if (HasAuthority())
	{
		AController* CurrentController = GetController();
		if (IsValid(CurrentController))
		{
			CurrentController->StopMovement();
			CurrentController->UnPossess();
		}
	}

	// 이 함수가 끝난 뒤, 자식 블루프린트의 OnDeath 이벤트(Ragdoll, 이펙트 재생 등)가 실행됩니다.
}

void ABaseJanggiPiece::SetCharacterState(EJanggiPieceState NewState)
{
	if (!HasAuthority() || CharacterState == NewState)
	{
		return;
	}

	EJanggiPieceState OldState = CharacterState;
	CharacterState = NewState;

	if (NewState != EJanggiPieceState::Dead)
	{
		bHasReportedDeath = false;
	}

	// 서버 측 상태 변경에 대한 처리 함수 호출
	OnRep_CharacterState(OldState);
}

void ABaseJanggiPiece::SetFaction(EJanggiFaction NewFaction)
{
	if (!HasAuthority() || Faction == NewFaction)
	{
		return;
	}

	Faction = NewFaction;
	UE_LOG(LogTemp, Log, TEXT("[BaseJanggiPiece] Faction assigned. Piece=%s Controller=%s Faction=%d"), *GetName(), *GetNameSafe(GetController()), static_cast<int32>(Faction));
}

void ABaseJanggiPiece::OnRep_CurrentHP(float OldHP)
{
	// 클라이언트에서 CurrentHP가 복제되었을 때 호출됨 (UI 체력바 갱신 등에 활용)
	const float DamageTaken = OldHP - CurrentHP;
	if (DamageTaken > 0.0f)
	{
		// 클라이언트 측 피격 이펙트 및 사운드 재생 이벤트 호출
		OnTakeDamageEvent(DamageTaken, CurrentHP, nullptr);
	}
}

void ABaseJanggiPiece::OnRep_CharacterState(EJanggiPieceState OldState)
{
	// 이동 가능 여부에 따라 MovementComponent 제어
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (IsValid(MoveComp))
	{
		if (CanMove())
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}
		else
		{
			MoveComp->DisableMovement();
		}
	}

	// 블루프린트 상태 변경 이벤트 훅 발동 (애니메이션, VFX 연동)
	OnStateChangedEvent(CharacterState, OldState);
}

bool ABaseJanggiPiece::CanReceiveInput() const
{
	return CanMove() || CanLook() || CanAttack() || CanUseSkill();
}

bool ABaseJanggiPiece::CanMove() const
{
	switch (CharacterState)
	{
	case EJanggiPieceState::Fighting:
		return true;
	case EJanggiPieceState::Preparing:
	case EJanggiPieceState::Stunned:
	case EJanggiPieceState::Dead:
	default:
		return false;
	}
}

bool ABaseJanggiPiece::CanLook() const
{
	switch (CharacterState)
	{
	case EJanggiPieceState::Fighting:
		return true;
	case EJanggiPieceState::Preparing:
	case EJanggiPieceState::Stunned:
	case EJanggiPieceState::Dead:
	default:
		return false;
	}
}

bool ABaseJanggiPiece::CanAttack() const
{
	switch (CharacterState)
	{
	case EJanggiPieceState::Fighting:
		return true;
	case EJanggiPieceState::Preparing:
	case EJanggiPieceState::Stunned:
	case EJanggiPieceState::Dead:
	default:
		return false;
	}
}

bool ABaseJanggiPiece::CanUseSkill() const
{
	switch (CharacterState)
	{
	case EJanggiPieceState::Fighting:
		return true;
	case EJanggiPieceState::Preparing:
	case EJanggiPieceState::Stunned:
	case EJanggiPieceState::Dead:
	default:
		return false;
	}
}
