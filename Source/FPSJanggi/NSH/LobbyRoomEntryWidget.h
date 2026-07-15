// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "SessionSubsystem.h"
#include "LobbyRoomEntryWidget.generated.h"

/** One searchable Steam room row with its own join action. */
UCLASS()
class FPSJANGGI_API ULobbyRoomEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeRoom(const FNSHRoomInfo& InRoom);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UFUNCTION()
	void JoinRoom();

	FNSHRoomInfo RoomInfo;
};
