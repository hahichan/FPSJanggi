// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JanggiGameInstance.h"
#include "ArenaGameMode.generated.h"

class ABaseJanggiPiece;
class AActor;
class AController;
class APlayerController;
class APawn;

/** 아레나 전투 페이즈 */
UENUM(BlueprintType)
enum class EArenaMatchPhase : uint8
{
	WaitingForPlayers	UMETA(DisplayName = "Waiting For Players"),
	Countdown			UMETA(DisplayName = "Countdown"),
	Fighting			UMETA(DisplayName = "Fighting"),
	MatchEnded			UMETA(DisplayName = "Match Ended")
};

UCLASS()
class JANGGBASE_API AArenaGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AArenaGameMode();

protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;

	// -------------------------------------------------------------------------
	// 전투 아레나 설정 및 상태 변수
	// -------------------------------------------------------------------------
protected:
	/** 카운트다운 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena|Rules", meta = (ClampMin = "0.0"))
	float CountdownDuration;

	/** 전투 종료 후 보드 레벨로 돌아가기 전 대기 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena|Rules", meta = (ClampMin = "0.0"))
	float PostMatchDelay;

	/** 복귀할 보드 레벨의 맵 경로 (예: "/Game/Maps/BoardLevel") */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena|Rules")
	FString BoardLevelMapPath;

	/** Player 0(공격자) 스폰에 사용할 폰 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena|PlayerSpawn")
	TSubclassOf<ABaseJanggiPiece> AttackerPawnClass;

	/** Player 1(수비자) 스폰에 사용할 폰 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena|PlayerSpawn")
	TSubclassOf<ABaseJanggiPiece> DefenderPawnClass;

	/** 구버전 BP 호환용: 기존 Test Piece Class A 값을 읽기 위한 폴백 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena|TestSpawn")
	TSubclassOf<ABaseJanggiPiece> TestPieceClassA;

	/** 구버전 BP 호환용: 기존 Test Piece Class B 값을 읽기 위한 폴백 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena|TestSpawn")
	TSubclassOf<ABaseJanggiPiece> TestPieceClassB;

	/** 랜덤 스폰 포인트로 사용할 액터 태그 (PlayerStart/TargetPoint 공통) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena|PlayerSpawn")
	FName ArenaSpawnTag;

	/** 현재 매치 페이즈 (서버 전용 상태 관리) */
	UPROPERTY(BlueprintReadOnly, Category = "Arena|State")
	EArenaMatchPhase CurrentMatchPhase;

	/** GameInstance에서 읽어온 전투 요청 데이터 캐시 */
	UPROPERTY(BlueprintReadOnly, Category = "Arena|State")
	FDuelRequestData ActiveRequestData;

	/** 연결 및 재매칭이 완료된 공격자 플레이어 컨트롤러 */
	UPROPERTY(BlueprintReadOnly, Category = "Arena|Players")
	TObjectPtr<APlayerController> AttackerController;

	/** 연결 및 재매칭이 완료된 방어자 플레이어 컨트롤러 */
	UPROPERTY(BlueprintReadOnly, Category = "Arena|Players")
	TObjectPtr<APlayerController> DefenderController;

	/** 스폰된 공격자 기물 액터 */
	UPROPERTY(BlueprintReadOnly, Category = "Arena|Players")
	TObjectPtr<ABaseJanggiPiece> AttackerPiece;

	/** 스폰된 방어자 기물 액터 */
	UPROPERTY(BlueprintReadOnly, Category = "Arena|Players")
	TObjectPtr<ABaseJanggiPiece> DefenderPiece;

	/** 카운트다운 타이머 핸들 */
	FTimerHandle TimerHandle_Countdown;

	/** 전투 종료 복귀 타이머 핸들 */
	FTimerHandle TimerHandle_ReturnToBoard;

	/** 수동 플레이어 스폰 재시도 타이머 핸들 */
	FTimerHandle TimerHandle_ManualSpawnRetry;

	/** 수동 스폰 재시도 간격(초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arena|PlayerSpawn", meta = (ClampMin = "0.05"))
	float ManualSpawnRetryInterval;

	/** 플레이어 폰 스폰이 이미 완료됐는지 여부 */
	bool bHasSpawnedPlayerPawns;

	// -------------------------------------------------------------------------
	// 아레나 핵심 로직 (서버 Authoritative)
	// -------------------------------------------------------------------------
protected:
	/** PlayerStart/TargetPoint 기반으로 두 플레이어 폰을 서버에서 스폰/빙의 처리 */
	virtual void TrySpawnPlayerPawns();

	/** 수동 스폰 재시도 타이머 시작 */
	virtual void StartManualSpawnRetry();

	/** 수동 스폰 재시도 타이머 중지 */
	virtual void StopManualSpawnRetry();

	/** PlayerStart/TargetPoint 중 ArenaSpawnTag가 붙은 스폰 포인트를 모두 수집 */
	virtual void GatherArenaSpawnPoints(TArray<AActor*>& OutSpawnPoints) const;

	/** 수집된 스폰 포인트 중 중복 없이 2개를 랜덤 선택 */
	virtual bool SelectTwoRandomSpawnPoints(const TArray<AActor*>& SpawnPoints, AActor*& OutStartPoint0, AActor*& OutStartPoint1) const;

	/** 서버에서 특정 플레이어에게 지정 클래스의 폰을 스폰하고 Possess 처리 */
	virtual ABaseJanggiPiece* SpawnPlayerPawnForController(APlayerController* PlayerController, TSubclassOf<ABaseJanggiPiece> PieceClass, AActor* SpawnPoint, const TCHAR* SpawnPointLabel, EJanggiFaction AssignedFaction, FName RoleTag);

	/** 게임 인스턴스에서 요청 데이터를 읽고 유효성 검증 */
	virtual bool LoadDuelRequestFromInstance();

	/** 입장한 플레이어 컨트롤러들을 ID 기준으로 재매칭 및 검증 */
	virtual void TryMatchmakingAndSpawn();

	/** 기물 캐릭터를 스폰하고 플레이어 컨트롤러에 Possess 처리 */
	virtual ABaseJanggiPiece* SpawnAndPossessPiece(APlayerController* PlayerController, TSoftClassPtr<ABaseJanggiPiece> PieceClass, FName StartTag, EJanggiFaction AssignedFaction);

	/** 태그에 맞는 PlayerStart 액터 검색 (제안: "Attacker" / "Defender" 태그 사용) */
	virtual AActor* FindPlayerStartByTag(FName StartTag);

	/** 카운트다운 시작 */
	virtual void StartCountdown();

	/** 카운트다운 완료 및 전투 시작 처리 */
	virtual void OnCountdownFinished();

	/** 전투 종료 및 결과 처리 (승패 / 무승부) */
	virtual void EndMatch(EDuelResult Result, const FString& WinnerId, const FString& LoserId);

	/** 보드 레벨로 ServerTravel 복귀 */
	virtual void ReturnToBoardLevel();

public:
	/** 캐릭터 사망 보고 함수 (ABaseJanggiPiece의 사망 이벤트나 델리게이트에서 호출) */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Arena|Rule")
	virtual void ReportPieceDeath(ABaseJanggiPiece* DeadPiece, AActor* Killer);

	// -------------------------------------------------------------------------
	// 블루프린트 확장 및 연출 통지 (UI, 사운드 등)
	// -------------------------------------------------------------------------
protected:
	/** 카운트다운 시작 시 HUD 연동 등을 위해 호출되는 BP 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Arena|Events", meta = (DisplayName = "On Countdown Started"))
	void OnCountdownStartedEvent(float Duration);

	/** 전투 시작 알림 BP 이벤트 ("Fight!" 연출 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Arena|Events", meta = (DisplayName = "On Fight Started"))
	void OnFightStartedEvent();

	/** 전투 종료 알림 BP 이벤트 (승리/패배/무승부 UI 연출) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Arena|Events", meta = (DisplayName = "On Match Ended"))
	void OnMatchEndedEvent(EDuelResult Result, APlayerController* WinnerController);
};
