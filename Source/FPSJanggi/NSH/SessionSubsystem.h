// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SessionSubsystem.generated.h"

class FOnlineSessionSearch;

USTRUCT(BlueprintType)
struct FPSJANGGI_API FNSHRoomInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "NSH|Session")
	int32 nsh_room_index = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "NSH|Session")
	FString nsh_room_name;

	UPROPERTY(BlueprintReadOnly, Category = "NSH|Session")
	FString nsh_host_name;

	UPROPERTY(BlueprintReadOnly, Category = "NSH|Session")
	FString nsh_session_id;

	UPROPERTY(BlueprintReadOnly, Category = "NSH|Session")
	int32 nsh_current_players = 0;

	UPROPERTY(BlueprintReadOnly, Category = "NSH|Session")
	int32 nsh_max_players = 0;

	UPROPERTY(BlueprintReadOnly, Category = "NSH|Session")
	int32 nsh_ping_ms = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNSHOnSessionOperationComplete, FName, nsh_operation, bool, nsh_b_success, const FString&, nsh_message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNSHOnRoomSearchComplete, bool, nsh_b_success, const TArray<FNSHRoomInfo>&, nsh_rooms);

UCLASS()
class FPSJANGGI_API USessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& nsh_collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "NSH|Session")
	FNSHOnSessionOperationComplete nsh_on_session_operation_complete;

	UPROPERTY(BlueprintAssignable, Category = "NSH|Session")
	FNSHOnRoomSearchComplete nsh_on_room_search_complete;

	UFUNCTION(BlueprintCallable, Category = "NSH|Session")
	void CreateRoom(const FString& nsh_room_name, int32 nsh_max_players = 2, bool nsh_b_is_lan = false);

	UFUNCTION(BlueprintCallable, Category = "NSH|Session")
	void FindRooms(int32 nsh_max_results = 50, bool nsh_b_is_lan = false);

	UFUNCTION(BlueprintCallable, Category = "NSH|Session")
	void JoinRoom(int32 nsh_room_index);

	UFUNCTION(BlueprintCallable, Category = "NSH|Session")
	void QuickMatch(int32 nsh_max_players = 2, bool nsh_b_is_lan = false);

	UFUNCTION(BlueprintCallable, Category = "NSH|Session")
	void LeaveRoom();

	UFUNCTION(BlueprintPure, Category = "NSH|Session")
	bool IsBusy() const { return nsh_b_operation_in_progress; }

	UFUNCTION(BlueprintPure, Category = "NSH|Session")
	TArray<FNSHRoomInfo> GetCachedRooms() const { return nsh_cached_rooms; }

private:
	static const FName nsh_session_name;
	static const FName nsh_room_name_key;
	static const FName nsh_game_key;

	IOnlineSessionPtr nsh_session_interface;
	TSharedPtr<FOnlineSessionSearch> nsh_session_search;
	TArray<FNSHRoomInfo> nsh_cached_rooms;

	FDelegateHandle nsh_create_delegate_handle;
	FDelegateHandle nsh_find_delegate_handle;
	FDelegateHandle nsh_join_delegate_handle;
	FDelegateHandle nsh_destroy_delegate_handle;

	bool nsh_b_operation_in_progress = false;
	bool nsh_b_delegates_bound = false;
	bool nsh_b_quick_match_pending = false;
	bool nsh_b_create_after_destroy = false;
	bool nsh_b_join_after_destroy = false;
	bool nsh_b_pending_is_lan = false;
	int32 nsh_pending_max_players = 2;
	int32 nsh_pending_join_index = INDEX_NONE;
	FString nsh_pending_room_name;

	bool EnsureSessionInterface(FName nsh_operation);
	void BindSessionDelegates();
	void BeginCreateRoom();
	void BeginJoinRoom(int32 nsh_room_index);
	void BroadcastFailure(FName nsh_operation, const FString& nsh_message);
	void ResetOperationState();

	void HandleCreateSessionComplete(FName nsh_completed_session_name, bool nsh_b_was_successful);
	void HandleFindSessionsComplete(bool nsh_b_was_successful);
	void HandleJoinSessionComplete(FName nsh_completed_session_name, EOnJoinSessionCompleteResult::Type nsh_result);
	void HandleDestroySessionComplete(FName nsh_completed_session_name, bool nsh_b_was_successful);
};
