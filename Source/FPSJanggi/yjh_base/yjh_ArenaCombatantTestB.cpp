// Copyright Epic Games, Inc. All Rights Reserved.

#include "yjh_base/yjh_ArenaCombatantTestB.h"

AYJHArenaCombatantTestB::AYJHArenaCombatantTestB()
{
	CombatantId = FName(TEXT("test_b"));
	PieceInstanceId = FName(TEXT("test_b"));
	TeamInfo = EYJHTeamInfo::Red;
	VisualScale = 1.08f;
	HitboxScale = 1.15f;
	MaxHP = 135.0f;
	BaseWalkSpeed = 600.0f;
	BasicAttackDamage = 30.0f;
	BasicAttackRange = 235.0f;
	BasicAttackRadius = 74.0f;
	BasicAttackCooldownSeconds = 0.38f;
}
