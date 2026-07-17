// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CommandSkillTypes.h"
#include "CommandSkillComponent.generated.h"

class ABaseJanggiPiece;
class UEnhancedInputComponent;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCommandSkillInputReceived, int32, SlotIndex, FName, InputActionName, FName, MatchedSkillId);

UCLASS(ClassGroup=(Janggi), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class JANGGBASE_API UCommandSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCommandSkillComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** Enhanced Input 액션 바인딩 */
	UFUNCTION(BlueprintCallable, Category = "CommandSkill|Input")
	void BindInput(UEnhancedInputComponent* EnhancedInputComponent);

	/** 슬롯으로 스킬 실행 요청 */
	UFUNCTION(BlueprintCallable, Category = "CommandSkill|Command")
	void TryUseSkillBySlot(ECommandSkillSlot Slot);

	/** 태그로 스킬 실행 요청 */
	UFUNCTION(BlueprintCallable, Category = "CommandSkill|Command")
	void TryUseSkillByTag(FName InputTag);

	/** 자기 즉사 테스트 커맨드 */
	UFUNCTION(BlueprintCallable, Category = "CommandSkill|Command")
	void TryUseSuicideCommand();

	/** 상대 즉사 테스트 커맨드 */
	UFUNCTION(BlueprintCallable, Category = "CommandSkill|Command")
	void TryUseKillEnemyCommand();

	/** 이동속도 변경 테스트 커맨드 */
	UFUNCTION(BlueprintCallable, Category = "CommandSkill|Command")
	void TryUseSpeedBuffCommand();

	/** 가속도 변경 테스트 커맨드 */
	UFUNCTION(BlueprintCallable, Category = "CommandSkill|Command")
	void TryUseAccelBuffCommand();

protected:
	/** 슬롯 1 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommandSkill|Input")
	TObjectPtr<UInputAction> SkillSlot1Action;

	/** 슬롯 2 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommandSkill|Input")
	TObjectPtr<UInputAction> SkillSlot2Action;

	/** 슬롯 3 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommandSkill|Input")
	TObjectPtr<UInputAction> SkillSlot3Action;

	/** 슬롯 4 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommandSkill|Input")
	TObjectPtr<UInputAction> SkillSlot4Action;

	/** 슬롯 5 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommandSkill|Input")
	TObjectPtr<UInputAction> SkillSlot5Action;

	/** 슬롯 6 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommandSkill|Input")
	TObjectPtr<UInputAction> SkillSlot6Action;

	/** 슬롯 7 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommandSkill|Input")
	TObjectPtr<UInputAction> SkillSlot7Action;

	/** 슬롯 8 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommandSkill|Input")
	TObjectPtr<UInputAction> SkillSlot8Action;

	/** 슬롯 9 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommandSkill|Input")
	TObjectPtr<UInputAction> SkillSlot9Action;

	/** 슬롯 10 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CommandSkill|Input")
	TObjectPtr<UInputAction> SkillSlot10Action;

	/** 캐릭터 BP에서 스킬을 추가/삭제하는 핵심 데이터 배열 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommandSkill|Data")
	TArray<FCommandSkillSpec> Skills;

	/** 테스트용 기본 스킬 세트를 자동으로 채울지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommandSkill|Data")
	bool bUseDefaultTestSkillSet;

	/** 구형 BP의 즉사/이속/가속 래퍼 커맨드 허용 여부(기본 비활성) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommandSkill|Data")
	bool bEnableLegacyCommandWrappers;

	/** BP에서 바로 바인딩 가능한 입력 수신 이벤트(슬롯 입력이 들어오면 항상 브로드캐스트) */
	UPROPERTY(BlueprintAssignable, Category = "CommandSkill|Events")
	FOnCommandSkillInputReceived OnCommandSkillInputReceived;

	UFUNCTION(Server, Reliable)
	void ServerUseSkillById(FName SkillRuntimeId);

private:
	TWeakObjectPtr<ABaseJanggiPiece> CachedOwnerPiece;
	TMap<FName, float> LastSkillUseServerTime;
	TMap<ECommandSkillSlot, float> LastSlotInputTime;

	/** Triggered 바인딩 중복 호출 억제 간격(초) */
	float SlotInputDebounceSeconds;

	/** InputAction 이름 -> 스킬 RuntimeId 캐시 */
	TMap<FName, FName> ActionNameToRuntimeSkillIdCache;

	/** 매칭 실패한 InputAction 이름 캐시 */
	TSet<FName> UnmatchedActionNameCache;

	ABaseJanggiPiece* ResolveOwnerPiece() const;
	ABaseJanggiPiece* ResolveEnemyTarget() const;
	bool CanExecuteCommand(const TCHAR* CommandName) const;
	bool CanExecuteSkillSpec(const FCommandSkillSpec& SkillSpec, FString& OutReason) const;
	FName ResolveSkillRuntimeId(const FCommandSkillSpec& SkillSpec) const;
	const FCommandSkillSpec* FindSkillByRuntimeId(FName SkillRuntimeId) const;
	const FCommandSkillSpec* FindSkillBySlot(ECommandSkillSlot Slot) const;
	const FCommandSkillSpec* FindSkillByTag(FName InputTag) const;
	const FCommandSkillSpec* FindFirstSkillByType(ECommandSkillType SkillType) const;
	const FCommandSkillSpec* FindSkillByAction(const UInputAction* InputAction) const;
	bool IsSkillOnCooldown(const FCommandSkillSpec& SkillSpec, float Now, float& OutRemaining) const;
	void ValidateSkillSpecs() const;
	void TryUseSkillByAssignedAction(const UInputAction* InputAction, ECommandSkillSlot FallbackSlot);
	void InitializeDefaultTestSkillsIfNeeded();
	void PrewarmActionSkillCache();
	bool ShouldProcessSlotInput(ECommandSkillSlot Slot);

	bool ExecuteSkillSpecServer(const FCommandSkillSpec& SkillSpec);
	bool ExecuteSuicideSkillServer(const FCommandSkillSpec& SkillSpec);
	bool ExecuteKillEnemySkillServer(const FCommandSkillSpec& SkillSpec);
	bool ExecuteModifyMoveSpeedSkillServer(const FCommandSkillSpec& SkillSpec);
	bool ExecuteModifyAccelerationSkillServer(const FCommandSkillSpec& SkillSpec);

	void OnSkillSlot1Triggered();
	void OnSkillSlot2Triggered();
	void OnSkillSlot3Triggered();
	void OnSkillSlot4Triggered();
	void OnSkillSlot5Triggered();
	void OnSkillSlot6Triggered();
	void OnSkillSlot7Triggered();
	void OnSkillSlot8Triggered();
	void OnSkillSlot9Triggered();
	void OnSkillSlot10Triggered();
};
