// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "yjh_ArenaSubStateMachine.generated.h"

UENUM()
enum class EYJHArenaSubState : uint8
{
	None,
	ArenaBattle,
	ArenaBattleReal
};

/**
 * Lightweight sub-state machine for YJH arena flow inside NSH's ArenaBattle phase.
 */
class FPSJANGGI_API FYJHArenaSubStateMachine
{
public:
	void Reset();
	void EnterArenaBattle();
	bool TryEnterArenaBattleReal(FString& OutFailureReason);
	void ReturnToArenaBattle();

	EYJHArenaSubState GetCurrentState() const { return CurrentState; }
	bool IsArenaBattleReal() const { return CurrentState == EYJHArenaSubState::ArenaBattleReal; }

private:
	EYJHArenaSubState CurrentState = EYJHArenaSubState::None;
};
