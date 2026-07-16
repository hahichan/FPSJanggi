// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseJanggiPiece.h"
#include "CommandSkillTypes.generated.h"

UENUM(BlueprintType)
enum class ECommandSkillSlot : uint8
{
	None	UMETA(DisplayName = "None"),
	Slot1	UMETA(DisplayName = "Slot 1"),
	Slot2	UMETA(DisplayName = "Slot 2"),
	Slot3	UMETA(DisplayName = "Slot 3"),
	Slot4	UMETA(DisplayName = "Slot 4"),
	Slot5	UMETA(DisplayName = "Slot 5"),
	Slot6	UMETA(DisplayName = "Slot 6"),
	Slot7	UMETA(DisplayName = "Slot 7"),
	Slot8	UMETA(DisplayName = "Slot 8"),
	Slot9	UMETA(DisplayName = "Slot 9"),
	Slot10	UMETA(DisplayName = "Slot 10")
};

UENUM(BlueprintType)
enum class ECommandSkillType : uint8
{
	None				UMETA(DisplayName = "None"),
	Suicide			UMETA(DisplayName = "Suicide"),
	KillEnemy		UMETA(DisplayName = "Kill Enemy"),
	ModifyMoveSpeed	UMETA(DisplayName = "Modify Move Speed"),
	ModifyAcceleration UMETA(DisplayName = "Modify Acceleration")
};

USTRUCT(BlueprintType)
struct FCommandSkillSpec
{
	GENERATED_BODY()

	/** 런타임 식별자(중복 금지 권장) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommandSkill|Skill")
	FName SkillId = NAME_None;

	/** 블루프린트 가독성을 위한 표시 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommandSkill|Skill")
	FText DisplayName;

	/** 팀 협업 친화용 기본 입력 슬롯 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommandSkill|Input")
	ECommandSkillSlot InputSlot = ECommandSkillSlot::Slot1;

	/** 추후 태그 기반 확장을 위한 보조 입력 키 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommandSkill|Input")
	FName InputTag = NAME_None;

	/** 스킬 실행 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommandSkill|Skill")
	ECommandSkillType SkillType = ECommandSkillType::None;

	/** 사용 가능 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommandSkill|Skill")
	bool bEnabled = true;

	/** 허용 상태 목록을 사용할지 여부 (false면 Owner->CanUseSkill() 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommandSkill|Rule")
	bool bUseAllowedStates = false;

	/** 허용 상태 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommandSkill|Rule")
	TArray<EJanggiPieceState> AllowedStates;

	/** 주 수치(데미지, 이동속도, 가속도 등 타입별 의미가 다름) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommandSkill|Tuning")
	float Magnitude = 0.0f;

	/** 보조 수치(예: 가속도 스킬의 BrakingDeceleration) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommandSkill|Tuning")
	float SecondaryMagnitude = 0.0f;

	/** 지속시간(현재 테스트 단계에서는 로깅/예약 값으로 유지) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommandSkill|Tuning", meta = (ClampMin = "0.0"))
	float Duration = 0.0f;

	/** 재사용 대기시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CommandSkill|Tuning", meta = (ClampMin = "0.0"))
	float Cooldown = 0.0f;
};
