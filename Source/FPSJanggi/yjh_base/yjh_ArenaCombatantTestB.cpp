// Copyright Epic Games, Inc. All Rights Reserved.

#include "yjh_base/yjh_ArenaCombatantTestB.h"

AYJHArenaCombatantTestB::AYJHArenaCombatantTestB()
{
	CombatantId = FName(TEXT("test_b"));
	PieceInstanceId = FName(TEXT("test_b"));
	TeamInfo = EYJHTeamInfo::Red;
	BaseWalkSpeed = 640.0f;
	BasicAttackDamage = 24.0f;
	BasicAttackRange = 240.0f;
	BasicAttackRadius = 70.0f;
	BasicAttackCooldownSeconds = 0.33f;
}
