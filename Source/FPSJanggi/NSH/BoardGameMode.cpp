// Copyright Epic Games, Inc. All Rights Reserved.

#include "BoardGameMode.h"

#include "Engine/World.h"
#include "BoardPlayerController.h"
#include "AuthoritativeJanggiBoard.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

ABoardGameMode::ABoardGameMode()
{
	PlayerControllerClass = ABoardPlayerController::StaticClass();
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
	if (AssignedTeam == EJanggiTeam::Red && FParse::Param(FCommandLine::Get(), TEXT("GeneralSmokeTest")))
	{
		FTimerHandle GeneralTestTimer;
		GetWorldTimerManager().SetTimer(GeneralTestTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (ABoardPlayerController* Host = Cast<ABoardPlayerController>(GetWorld()->GetFirstPlayerController()))
			{
				Host->RunGeneralDefeatSmokeTest();
			}
		}), 2.0f, false);
	}
#endif
}
