// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseJanggiPiece.generated.h"

class AController;
class AActor;
class UInputComponent;
class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class USkeletalMeshComponent;
class UCommandSkillComponent;
struct FDamageEvent;

struct FInputActionValue;

/** 장기 기물의 진영 */
UENUM(BlueprintType)
enum class EJanggiFaction : uint8
{
	None	UMETA(DisplayName = "None"),
	Red		UMETA(DisplayName = "Red (Cho / 한)"),
	Blue	UMETA(DisplayName = "Blue (Han / 초)")
};

/** 장기 기물 캐릭터의 현재 전투 상태 */
UENUM(BlueprintType)
enum class EJanggiPieceState : uint8
{
	Preparing	UMETA(DisplayName = "Preparing"),	// 라운드 준비 중 (이동/공격 불가)
	Fighting	UMETA(DisplayName = "Fighting"),	// 전투 중 (정상 동작)
	Stunned		UMETA(DisplayName = "Stunned"),		// 기절/경직 (이동/공격 불가)
	Dead		UMETA(DisplayName = "Dead")			// 사망 (입력 불가)
};

UCLASS(Abstract)
class JANGGBASE_API ABaseJanggiPiece : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseJanggiPiece();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** 언리얼 표준 데미지 처리 인터페이스 오버라이드 (서버에서만 호출됨) */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// -------------------------------------------------------------------------
	// 스탯 및 규칙 설정 (Blueprint에서 상속 후 수정할 영역)
	// -------------------------------------------------------------------------
protected:
	/** 최대 체력 (클래스 디폴트에서 설정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Health", meta = (ClampMin = "1.0"))
	float MaxHP;

	/** 초당 체력 재생량 (서버에서만 적용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Health", meta = (ClampMin = "0.0"))
	float HPRegenPerSecond;

	/** 자동 체력 재생 사용 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Health")
	bool bUseAutoRegen;

	/** 피격 후 체력 재생 시작까지의 지연 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Health", meta = (ClampMin = "0.0"))
	float RegenDelayAfterHit;

	/** 최대 이동 속도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Movement", meta = (ClampMin = "0.0"))
	float MaxWalkSpeed;

	/** 최대 가속도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Movement", meta = (ClampMin = "0.0"))
	float MaxAcceleration;

	/** 보행 감속도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Movement", meta = (ClampMin = "0.0"))
	float BrakingDecelerationWalking;

	/** 지면 마찰 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Movement", meta = (ClampMin = "0.0"))
	float GroundFriction;

	/** 회전 속도(초당 각속도) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Movement")
	FRotator RotationRate;

	/** 공중 제어력 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AirControl;

	/** 점프 속도 (점프 시스템 확장 대비) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Movement", meta = (ClampMin = "0.0"))
	float JumpZVelocity;

	/** 기본 공격 데미지 (근접 시스템 확장용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Combat", meta = (ClampMin = "0.0"))
	float BaseAttackDamage;

	/** 공격 사거리 (근접 판정 반경/거리 계산용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Combat", meta = (ClampMin = "0.0"))
	float AttackRange;

	/** 피격 판정 반경(근접 히트스캔 보조용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Combat", meta = (ClampMin = "0.0"))
	float HitRadius;

	/** 공격 쿨다운 (근접 시스템 확장용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Combat", meta = (ClampMin = "0.0"))
	float AttackCooldown;

	/** 피격 경직 시간 (확장용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Combat", meta = (ClampMin = "0.0"))
	float HitStunDuration;

	/** 넉백 강도 (확장용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Combat", meta = (ClampMin = "0.0"))
	float KnockbackStrength;

	/** 피격 판정 크기 스케일 (캡슐 크기 배율) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|HitResistance", meta = (ClampMin = "0.1"))
	float HitboxScale;

	/** 받는 데미지 배율 (1.0 = 기본) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|HitResistance", meta = (ClampMin = "0.0"))
	float DamageMultiplier;

	/** 경직 저항 (0~1, 확장용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|HitResistance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StunResistance;

	/** 둔화 저항 (0~1, 확장용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|HitResistance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SlowResistance;

	/** 넉백 저항 (0~1, 확장용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|HitResistance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float KnockbackResistance;

	/** 테스트용 기본 입력 매핑 컨텍스트 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Input")
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;

	/** WASD 이동용 입력 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Input")
	TObjectPtr<UInputAction> MoveAction;

	/** 마우스 시야 회전용 입력 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Input")
	TObjectPtr<UInputAction> LookAction;

	/** 부모 클래스에서 공통 제공하는 1인칭 카메라 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JanggiPiece|FirstPerson")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	/** 1인칭 팔/손 전용 메쉬 (필요 시 자식 BP에서 자산 지정) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JanggiPiece|FirstPerson")
	TObjectPtr<USkeletalMeshComponent> FirstPersonArmsMesh;

	/** 소속 진영 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "JanggiPiece|Stats")
	EJanggiFaction Faction;

	/** 테스트/디버그용 커맨드 스킬 실행 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JanggiPiece|CommandSkill")
	TObjectPtr<UCommandSkillComponent> CommandSkillComponent;

	// -------------------------------------------------------------------------
	// 런타임 복제 변수 (서버 Authoritative)
	// -------------------------------------------------------------------------
protected:
	/** 현재 체력 */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, BlueprintReadOnly, Category = "JanggiPiece|Runtime")
	float CurrentHP;

	/** 현재 캐릭터 상태 (입력 제어 및 모션 판정용) */
	UPROPERTY(ReplicatedUsing = OnRep_CharacterState, BlueprintReadOnly, Category = "JanggiPiece|Runtime")
	EJanggiPieceState CharacterState;

	/** 서버에서 사망 보고가 이미 전달됐는지 여부 (중복 EndMatch 방지용) */
	UPROPERTY(Transient)
	bool bHasReportedDeath;

	/** 서버 기준 마지막 피격 시간(자동 재생 지연 계산용) */
	UPROPERTY(Transient)
	float LastTimeDamaged;

	/** 캡슐 기본 반지름 (HitboxScale 적용 기준값) */
	UPROPERTY(Transient)
	float BaseCapsuleRadius;

	/** 캡슐 기본 반높이 (HitboxScale 적용 기준값) */
	UPROPERTY(Transient)
	float BaseCapsuleHalfHeight;

	// -------------------------------------------------------------------------
	// Replication OnRep 함수
	// -------------------------------------------------------------------------
protected:
	UFUNCTION()
	virtual void OnRep_CurrentHP(float OldHP);

	UFUNCTION()
	virtual void OnRep_CharacterState(EJanggiPieceState OldState);

	// -------------------------------------------------------------------------
	// 전투 및 규칙 판정 로직 (서버 전용 및 공통)
	// -------------------------------------------------------------------------
public:
	/** 현재 입력을 받을 수 있는 상태인지 확인 (이동, 공격 시 블루프린트/C++에서 체크) */
	UFUNCTION(BlueprintPure, Category = "JanggiPiece|Rule")
	virtual bool CanReceiveInput() const;

	/** 상태 기반 이동 가능 여부 */
	UFUNCTION(BlueprintPure, Category = "JanggiPiece|Rule")
	virtual bool CanMove() const;

	/** 상태 기반 시야 회전 가능 여부 */
	UFUNCTION(BlueprintPure, Category = "JanggiPiece|Rule")
	virtual bool CanLook() const;

	/** 상태 기반 기본 공격 가능 여부 */
	UFUNCTION(BlueprintPure, Category = "JanggiPiece|Rule")
	virtual bool CanAttack() const;

	/** 상태 기반 스킬 사용 가능 여부 */
	UFUNCTION(BlueprintPure, Category = "JanggiPiece|Rule")
	virtual bool CanUseSkill() const;

	/** 현재 상태 조회 */
	UFUNCTION(BlueprintPure, Category = "JanggiPiece|Rule")
	EJanggiPieceState GetCharacterState() const { return CharacterState; }

	/** 진영 확인 */
	UFUNCTION(BlueprintPure, Category = "JanggiPiece|Rule")
	EJanggiFaction GetFaction() const { return Faction; }

	/** 현재 체력 조회 (BP 전용 스킬/위젯에서 사용) */
	UFUNCTION(BlueprintPure, Category = "JanggiPiece|Health")
	float GetCurrentHP() const { return CurrentHP; }

	/** 최대 체력 조회 (BP 전용 스킬/위젯에서 사용) */
	UFUNCTION(BlueprintPure, Category = "JanggiPiece|Health")
	float GetMaxHP() const { return MaxHP; }

	/** 현재 체력 비율(0~1) 조회 */
	UFUNCTION(BlueprintPure, Category = "JanggiPiece|Health")
	float GetHealthRatio() const { return MaxHP > 0.0f ? (CurrentHP / MaxHP) : 0.0f; }

	/** 1인칭 카메라 컴포넌트 접근 함수 */
	UFUNCTION(BlueprintPure, Category = "JanggiPiece|FirstPerson")
	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

	/** 1인칭 팔 메쉬 컴포넌트 접근 함수 */
	UFUNCTION(BlueprintPure, Category = "JanggiPiece|FirstPerson")
	USkeletalMeshComponent* GetFirstPersonArmsMesh() const { return FirstPersonArmsMesh; }

	/** 테스트용 커맨드 스킬 컴포넌트 접근 함수 */
	UFUNCTION(BlueprintPure, Category = "JanggiPiece|CommandSkill")
	UCommandSkillComponent* GetCommandSkillComponent() const { return CommandSkillComponent; }

	/** 상태 변경 함수 (서버에서만 호출해야 함) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "JanggiPiece|Rule")
	virtual void SetCharacterState(EJanggiPieceState NewState);

	/** 진영 변경 함수 (서버에서만 호출해야 함) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "JanggiPiece|Rule")
	virtual void SetFaction(EJanggiFaction NewFaction);

public:
	/** 커맨드 스킬 슬롯 입력이 들어왔을 때 BP에서 처리할 수 있는 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "JanggiPiece|CommandSkill", meta = (DisplayName = "On Command Skill Input"))
	void OnCommandSkillInputEvent(int32 SlotIndex, FName InputActionName, FName MatchedSkillId);

protected:

	/** BP 입력이 깨졌을 때 대비한 런타임 보조 입력 생성 */
	void EnsureInputFallback();

	/** 보조 입력 초기화 여부 */
	UPROPERTY(Transient)
	bool bInputFallbackInitialized;

	/** 보조 입력 매핑 컨텍스트 */
	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> FallbackInputMappingContext;

	/** 보조 이동 액션 */
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> FallbackMoveAction;

	/** 보조 시야 액션 */
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> FallbackLookAction;

	/** 로컬 플레이어용 입력 매핑 컨텍스트 등록 */
	void AddDefaultInputMapping();

	/** 이동 입력 처리 */
	void Move(const FInputActionValue& Value);

	/** 회전 입력 처리 */
	void Look(const FInputActionValue& Value);

	/** 사망 처리 내부 함수 (서버에서 호출) */
	virtual void HandleDeath(AActor* Killer);

	/** 이동 관련 스탯을 CharacterMovement에 적용 */
	void ApplyMovementStatsToMovementComponent();

	/** 자동 체력 재생 처리(서버 전용) */
	void HandleAutoRegen(float DeltaTime);

	// -------------------------------------------------------------------------
	// 블루프린트 확장용 이벤트 훅 (UI, 이펙트, 사운드, 모션 연동용)
	// -------------------------------------------------------------------------
protected:
	/** 피격 시 이펙트/사운드/UI 갱신을 위해 BP에서 구현하는 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "JanggiPiece|Events", meta = (DisplayName = "On Take Damage"))
	void OnTakeDamageEvent(float DamageAmount, float CurrentHealth, AActor* DamageCauser);

	/** 상태 변경 시 애니메이션이나 VFX 연출을 위해 BP에서 구현하는 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "JanggiPiece|Events", meta = (DisplayName = "On State Changed"))
	void OnStateChangedEvent(EJanggiPieceState NewState, EJanggiPieceState OldState);

	/** 
	 * 사망 시 호출되는 이벤트. 
	 * BlueprintNativeEvent로 선언하여, C++에서 콜리전 해제 등 필수 작업을 수행하고 BP에서 사망 모션/래그돌/사운드를 연출합니다.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "JanggiPiece|Events", meta = (DisplayName = "On Death"))
	void OnDeath(AActor* Killer);
	virtual void OnDeath_Implementation(AActor* Killer);
};
