// Copyright Epic Games, Inc. All Rights Reserved.

#include "yjh_base/yjh_ArenaSubStateMachine.h"

void FYJHArenaSubStateMachine::Reset()
{
	CurrentState = EYJHArenaSubState::None;
}

void FYJHArenaSubStateMachine::EnterArenaBattle()
{
	CurrentState = EYJHArenaSubState::ArenaBattle;
}

bool FYJHArenaSubStateMachine::TryEnterArenaBattleReal(FString& OutFailureReason)
{
	if (CurrentState != EYJHArenaSubState::ArenaBattle)
	{
		OutFailureReason = TEXT("YJH Arena Start is only available during ArenaBattle.");
		return false;
	}

	CurrentState = EYJHArenaSubState::ArenaBattleReal;
	return true;
}

void FYJHArenaSubStateMachine::ReturnToArenaBattle()
{
	if (CurrentState == EYJHArenaSubState::ArenaBattleReal)
	{
		CurrentState = EYJHArenaSubState::ArenaBattle;
	}
}
