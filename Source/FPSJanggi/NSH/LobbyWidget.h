// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "SessionSubsystem.h"
#include "LobbyWidget.generated.h"

class UEditableTextBox;
class UTextBlock;
class UVerticalBox;

/** Standalone front-end for Steam room creation, browsing, joining and quick match. */
UCLASS()
class FPSJANGGI_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void CreateRoom();

	UFUNCTION()
	void RefreshRooms();

	UFUNCTION()
	void QuickMatch();

	UFUNCTION()
	void OpenLocalPreview();

	UFUNCTION()
	void HandleSessionOperation(FName Operation, bool bSuccess, const FString& Message);

	UFUNCTION()
	void HandleRoomSearch(bool bSuccess, const TArray<FNSHRoomInfo>& Rooms);

	USessionSubsystem* GetSessions() const;
	void PopulateRooms(const TArray<FNSHRoomInfo>& Rooms);
	void SetStatus(const FString& Message, bool bError = false);

	UPROPERTY(Transient)
	TObjectPtr<UEditableTextBox> RoomNameInput;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> RoomList;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	bool bDelegatesBound = false;
};
