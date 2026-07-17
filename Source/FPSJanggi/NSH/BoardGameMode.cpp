// Copyright Epic Games, Inc. All Rights Reserved.

#include "BoardGameMode.h"

#include "Engine/World.h"
#include "BoardPlayerController.h"
#include "AuthoritativeJanggiBoard.h"
#include "GameFramework/GameStateBase.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

ABoardGameMode::ABoardGameMode()
{
	PlayerControllerClass = ABoardPlayerController::StaticClass();
}

void ABoardGameMode::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(
		ArenaSessionMonitorTimerHandle,
		this,
		&ABoardGameMode::MonitorArenaBattleRealSession,
		0.25f,
		true);
}

void ABoardGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABoardPlayerController* JoinedController = Cast<ABoardPlayerController>(NewPlayer);
	if (!JoinedController)
	{
		return;
	}

	bool bBlueTaken = false;
	bool bRedTaken = false;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const ABoardPlayerController* ExistingController = Cast<ABoardPlayerController>(It->Get());
		if (!ExistingController || ExistingController == JoinedController)
		{
			continue;
		}

		bBlueTaken |= ExistingController->GetAssignedBoardTeam() == EJanggiTeam::Blue;
		bRedTaken |= ExistingController->GetAssignedBoardTeam() == EJanggiTeam::Red;
	}

	const EJanggiTeam AssignedTeam = !bBlueTaken
		? EJanggiTeam::Blue
		: !bRedTaken ? EJanggiTeam::Red : EJanggiTeam::Unassigned;
	JoinedController->SetAssignedBoardTeam(AssignedTeam);
	if (AssignedTeam != EJanggiTeam::Unassigned)
	{
		DisconnectDeadlineByTeam.Remove(AssignedTeam);
	}
#if !UE_BUILD_SHIPPING
	if (FParse::Param(FCommandLine::Get(), TEXT("BoardSmokeTest")))
	{
		JoinedController->ClientRunLateJoinSmokeCheck();
		if (AssignedTeam == EJanggiTeam::Red)
		{
			FTimerHandle TestTimer;
			GetWorldTimerManager().SetTimer(TestTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (ABoardPlayerController* Host = Cast<ABoardPlayerController>(GetWorld()->GetFirstPlayerController()))
				{
					Host->RunBoardSmokeTest();
				}
			}), 2.0f, false);
		}
	}
	if (AssignedTeam == EJanggiTeam::Red && FParse::Param(FCommandLine::Get(), TEXT("ArenaSmokeTest")))
	{
		UE_LOG(LogTemp, Display, TEXT("BOARD_ARENA_SMOKE_TEST scheduled"));
		FTimerHandle ArenaTestTimer;
		GetWorldTimerManager().SetTimer(ArenaTestTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (ABoardPlayerController* Host = Cast<ABoardPlayerController>(GetWorld()->GetFirstPlayerController()))
			{
				Host->RunArenaSmokeTest();
			}
		}), 2.0f, false);
	}
	const bool bReturnLobbySmokeTest = FParse::Param(FCommandLine::Get(), TEXT("ReturnLobbySmokeTest"));
	if (AssignedTeam == EJanggiTeam::Red &&
		(FParse::Param(FCommandLine::Get(), TEXT("GeneralSmokeTest")) || bReturnLobbySmokeTest))
	{
		FTimerHandle GeneralTestTimer;
		GetWorldTimerManager().SetTimer(GeneralTestTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (ABoardPlayerController* Host = Cast<ABoardPlayerController>(GetWorld()->GetFirstPlayerController()))
			{
				Host->RunGeneralDefeatSmokeTest();
			}
		}), 2.0f, false);

		if (bReturnLobbySmokeTest)
		{
			FTimerHandle ReturnLobbyTestTimer;
			GetWorldTimerManager().SetTimer(ReturnLobbyTestTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (ABoardPlayerController* Host = Cast<ABoardPlayerController>(GetWorld()->GetFirstPlayerController()))
				{
					UE_LOG(LogTemp, Display, TEXT("BOARD_RETURN_TO_LOBBY_SMOKE_TEST requesting"));
					Host->RequestReturnToLobby();
				}
			}), 4.0f, false);
		}
	}
#endif
}

void ABoardGameMode::Logout(AController* Exiting)
{
	ABoardPlayerController* ExitingController = Cast<ABoardPlayerController>(Exiting);
	const EJanggiTeam ExitingTeam = ExitingController ? ExitingController->GetAssignedBoardTeam() : EJanggiTeam::Unassigned;

	Super::Logout(Exiting);

	AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard();
	if (!Board || ExitingTeam == EJanggiTeam::Unassigned)
	{
		return;
	}

	const EBoardMatchPhase Phase = Board->GetMatchPhase();
	const bool bArenaActive = Phase == EBoardMatchPhase::ArenaTransition || Phase == EBoardMatchPhase::ArenaBattle;
	if (!bArenaActive)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const AGameStateBase* CurrentGameState = World ? World->GetGameState() : nullptr;
	const double Now = CurrentGameState ? CurrentGameState->GetServerWorldTimeSeconds() : (World ? World->GetTimeSeconds() : 0.0);
	DisconnectDeadlineByTeam.Add(ExitingTeam, Now + ArenaDisconnectGraceSeconds);

	UE_LOG(LogTemp, Warning, TEXT("YJH_ARENA_DISCONNECT_GRACE_START team=%s deadline=%.2f session=%s"),
		*UEnum::GetValueAsString(ExitingTeam),
		Now + ArenaDisconnectGraceSeconds,
		*Board->GetActiveCombatSessionId().ToString());
}

AAuthoritativeJanggiBoard* ABoardGameMode::FindAuthoritativeBoard() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	for (TActorIterator<AAuthoritativeJanggiBoard> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

void ABoardGameMode::MonitorArenaBattleRealSession()
{
	AAuthoritativeJanggiBoard* Board = FindAuthoritativeBoard();
	if (!Board)
	{
		return;
	}

	const EBoardMatchPhase Phase = Board->GetMatchPhase();
	const bool bArenaActive = Phase == EBoardMatchPhase::ArenaTransition || Phase == EBoardMatchPhase::ArenaBattle;
	if (!bArenaActive)
	{
		ObservedArenaSessionId = NAME_None;
		DisconnectDeadlineByTeam.Reset();
		return;
	}

	const FName ActiveSessionId = Board->GetActiveCombatSessionId();
	if (ObservedArenaSessionId != ActiveSessionId)
	{
		ObservedArenaSessionId = ActiveSessionId;
		DisconnectDeadlineByTeam.Reset();
	}

	TSet<EJanggiTeam> PresentTeams;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const ABoardPlayerController* Controller = Cast<ABoardPlayerController>(It->Get());
		if (!Controller)
		{
			continue;
		}
		const EJanggiTeam Team = Controller->GetAssignedBoardTeam();
		if (Team != EJanggiTeam::Unassigned)
		{
			PresentTeams.Add(Team);
		}
	}

	for (const EJanggiTeam Team : PresentTeams)
	{
		DisconnectDeadlineByTeam.Remove(Team);
	}

	if (DisconnectDeadlineByTeam.Num() == 0)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const AGameStateBase* CurrentGameState = World ? World->GetGameState() : nullptr;
	const double Now = CurrentGameState ? CurrentGameState->GetServerWorldTimeSeconds() : (World ? World->GetTimeSeconds() : 0.0);

	for (const TPair<EJanggiTeam, double>& Pair : DisconnectDeadlineByTeam)
	{
		if (Now >= Pair.Value)
		{
			UE_LOG(LogTemp, Error, TEXT("YJH_ARENA_FORCED_ABORT reason=DisconnectGraceExpired team=%s session=%s"),
				*UEnum::GetValueAsString(Pair.Key), *ActiveSessionId.ToString());
			Board->AbortArenaBattleForced(TEXT("DisconnectGraceExpired"));
			DisconnectDeadlineByTeam.Reset();
			return;
		}
	}
}
