// Copyright Epic Games, Inc. All Rights Reserved.

#include "yjh_base/yjh_ArenaCombatantTestA.h"

AYJHArenaCombatantTestA::AYJHArenaCombatantTestA()
{
	CombatantId = FName(TEXT("test_a"));
	PieceInstanceId = FName(TEXT("test_a"));
	TeamInfo = EYJHTeamInfo::Blue;
	BaseWalkSpeed = 640.0f;
	BasicAttackDamage = 24.0f;
	BasicAttackRange = 240.0f;
	BasicAttackRadius = 70.0f;
	BasicAttackCooldownSeconds = 0.33f;
}
