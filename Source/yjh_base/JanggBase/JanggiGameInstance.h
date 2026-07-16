// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BaseJanggiPiece.h"
#include "JanggiGameInstance.generated.h"

/** 전투 종료 결과 상태 */
UENUM(BlueprintType)
enum class EDuelResult : uint8
{
	None		UMETA(DisplayName = "None / Not Finished"),
	AttackerWin	UMETA(DisplayName = "Attacker Win"),
	DefenderWin	UMETA(DisplayName = "Defender Win"),
	Draw		UMETA(DisplayName = "Draw")
};

/** 
 * 전투 요청 데이터 (보드 레벨 -> 전투 아레나 레벨로 트래블 시 전달)
 * 주의: APlayerController 나 AActor 같은 월드 종속 포인터 절대 포함 금지!
 */
USTRUCT(BlueprintType)
struct FDuelRequestData
{
	GENERATED_BODY()

	/** 공격자 플레이어 고유 식별자 (예: UniqueNetId String 또는 플레이어 인덱스) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JanggiDuel|Request")
	FString AttackerPlayerId;

	/** 방어자 플레이어 고유 식별자 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JanggiDuel|Request")
	FString DefenderPlayerId;

	/** 공격자가 아레나에서 스폰하여 조종할 기물 클래스 (Soft Class Pointer로 메모리 절약 및 안전성 확보) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JanggiDuel|Request")
	TSoftClassPtr<ABaseJanggiPiece> AttackerPieceClass;

	/** 방어자가 아레나에서 스폰하여 조종할 기물 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JanggiDuel|Request")
	TSoftClassPtr<ABaseJanggiPiece> DefenderPieceClass;

	/** 장기판 상의 어느 좌표(또는 노드 ID)에서 일어난 전투인지 식별 (선택적 기능) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JanggiDuel|Request")
	int32 BoardCellIndex;

	FDuelRequestData()
		: AttackerPlayerId(TEXT(""))
		, DefenderPlayerId(TEXT(""))
		, AttackerPieceClass(nullptr)
		, DefenderPieceClass(nullptr)
		, BoardCellIndex(-1)
	{
	}

	/** 데이터 유효성 검사 */
	bool IsValid() const
	{
		return !AttackerPlayerId.IsEmpty() && !DefenderPlayerId.IsEmpty() && !AttackerPieceClass.IsNull() && !DefenderPieceClass.IsNull();
	}
};

/** 
 * 전투 결과 데이터 (전투 아레나 레벨 -> 보드 레벨로 트래블 시 전달)
 */
USTRUCT(BlueprintType)
struct FDuelResultData
{
	GENERATED_BODY()

	/** 최종 전투 승패 결과 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JanggiDuel|Result")
	EDuelResult Result;

	/** 전투가 일어났던 장기판 좌표 (결과를 적용할 위치) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JanggiDuel|Result")
	int32 BoardCellIndex;

	/** 승리한 플레이어 ID (무승부나 None일 경우 빈 문자열) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JanggiDuel|Result")
	FString WinnerPlayerId;

	/** 패배한 플레이어 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JanggiDuel|Result")
	FString LoserPlayerId;

	FDuelResultData()
		: Result(EDuelResult::None)
		, BoardCellIndex(-1)
		, WinnerPlayerId(TEXT(""))
		, LoserPlayerId(TEXT(""))
	{
	}
};

UCLASS()
class JANGGBASE_API UJanggiGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UJanggiGameInstance();

	virtual void Init() override;

protected:
	/** 아레나 트래블을 위해 캐싱된 전투 요청 데이터 */
	UPROPERTY(BlueprintReadOnly, Category = "JanggiDuel|Data")
	FDuelRequestData CachedRequestData;

	/** 보드 복귀를 위해 캐싱된 전투 결과 데이터 */
	UPROPERTY(BlueprintReadOnly, Category = "JanggiDuel|Data")
	FDuelResultData CachedResultData;

	// -------------------------------------------------------------------------
	// Blueprint Getter / Setter / Clear 함수
	// -------------------------------------------------------------------------
public:
	/** 전투 요청 데이터 저장 (전투 아레나 로드 직전 호출) */
	UFUNCTION(BlueprintCallable, Category = "JanggiDuel|Request")
	void SetDuelRequestData(const FDuelRequestData& InRequestData);

	/** 전투 요청 데이터 조회 */
	UFUNCTION(BlueprintPure, Category = "JanggiDuel|Request")
	FDuelRequestData GetDuelRequestData() const;

	/** 전투 요청 데이터 유효성 확인 */
	UFUNCTION(BlueprintPure, Category = "JanggiDuel|Request")
	bool HasValidDuelRequest() const;

	/** 전투 요청 데이터 비우기 (아레나 로드 완료 후 사용 처리 시 호출) */
	UFUNCTION(BlueprintCallable, Category = "JanggiDuel|Request")
	void ClearDuelRequestData();

	/** 전투 결과 데이터 저장 (아레나에서 전투 종료 시 호출) */
	UFUNCTION(BlueprintCallable, Category = "JanggiDuel|Result")
	void SetDuelResultData(const FDuelResultData& InResultData);

	/** 전투 결과 데이터 조회 */
	UFUNCTION(BlueprintPure, Category = "JanggiDuel|Result")
	FDuelResultData GetDuelResultData() const;

	/** 전투 결과 데이터 비우기 (보드 레벨에서 결과 반영 후 호출) */
	UFUNCTION(BlueprintCallable, Category = "JanggiDuel|Result")
	void ClearDuelResultData();

	/** 모든 캐시 데이터 초기화 */
	UFUNCTION(BlueprintCallable, Category = "JanggiDuel|Data")
	void ClearAllDuelData();
};
