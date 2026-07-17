// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BoardGameMode.generated.h"

enum class EJanggiTeam : uint8;

UCLASS(Blueprintable)
class FPSJANGGI_API ABoardGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABoardGameMode();
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS Janggi|Arena|Rules", meta = (ClampMin = "0.0"))
	float ArenaDisconnectGraceSeconds = 8.0f;

private:
	void MonitorArenaBattleRealSession();
	class AAuthoritativeJanggiBoard* FindAuthoritativeBoard() const;

	FTimerHandle ArenaSessionMonitorTimerHandle;
	FName ObservedArenaSessionId = NAME_None;
	TMap<EJanggiTeam, double> DisconnectDeadlineByTeam;
};
