// Copyright Epic Games, Inc. All Rights Reserved.

#include "yjh_base/yjh_ArenaCombatantTestA.h"

AYJHArenaCombatantTestA::AYJHArenaCombatantTestA()
{
	CombatantId = FName(TEXT("test_a"));
	PieceInstanceId = FName(TEXT("test_a"));
	TeamInfo = EYJHTeamInfo::Blue;
	VisualScale = 0.92f;
	HitboxScale = 0.86f;
	MaxHP = 105.0f;
	BaseWalkSpeed = 690.0f;
	BasicAttackDamage = 22.0f;
	BasicAttackRange = 255.0f;
	BasicAttackRadius = 64.0f;
	BasicAttackCooldownSeconds = 0.30f;
}
