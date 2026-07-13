// Copyright Epic Games, Inc. All Rights Reserved.

#include "SessionSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

const FName USessionSubsystem::nsh_session_name = NAME_GameSession;
const FName USessionSubsystem::nsh_room_name_key(TEXT("FPSJANGGI_ROOM_NAME"));
const FName USessionSubsystem::nsh_game_key(TEXT("FPSJANGGI_GAME"));

void USessionSubsystem::Initialize(FSubsystemCollectionBase& nsh_collection)
{
	Super::Initialize(nsh_collection);

	if (IOnlineSubsystem* nsh_online_subsystem = IOnlineSubsystem::Get())
	{
		nsh_session_interface = nsh_online_subsystem->GetSessionInterface();
	}

	BindSessionDelegates();
}

void USessionSubsystem::Deinitialize()
{
	if (nsh_session_interface.IsValid() && nsh_b_delegates_bound)
	{
		nsh_session_interface->ClearOnCreateSessionCompleteDelegate_Handle(nsh_create_delegate_handle);
		nsh_session_interface->ClearOnFindSessionsCompleteDelegate_Handle(nsh_find_delegate_handle);
		nsh_session_interface->ClearOnJoinSessionCompleteDelegate_Handle(nsh_join_delegate_handle);
		nsh_session_interface->ClearOnDestroySessionCompleteDelegate_Handle(nsh_destroy_delegate_handle);
	}

	nsh_b_delegates_bound = false;
	nsh_session_search.Reset();
	nsh_session_interface.Reset();
	Super::Deinitialize();
}

void USessionSubsystem::CreateRoom(const FString& nsh_room_name, int32 nsh_max_players, bool nsh_b_is_lan)
{
	if (nsh_b_operation_in_progress)
	{
		BroadcastFailure(TEXT("CreateRoom"), TEXT("Another session operation is already in progress."));
		return;
	}

	if (!EnsureSessionInterface(TEXT("CreateRoom")))
	{
		return;
	}

	nsh_pending_room_name = nsh_room_name.TrimStartAndEnd();
	if (nsh_pending_room_name.IsEmpty())
	{
		nsh_pending_room_name = TEXT("FPS Janggi Room");
	}

	nsh_pending_max_players = FMath::Clamp(nsh_max_players, 2, 8);
	nsh_b_pending_is_lan = nsh_b_is_lan;
	nsh_b_operation_in_progress = true;

	if (nsh_session_interface->GetNamedSession(nsh_session_name) != nullptr)
	{
		nsh_b_create_after_destroy = true;
		if (!nsh_session_interface->DestroySession(nsh_session_name))
		{
			ResetOperationState();
			BroadcastFailure(TEXT("CreateRoom"), TEXT("Could not replace the existing room."));
		}
		return;
	}

	BeginCreateRoom();
}

void USessionSubsystem::FindRooms(int32 nsh_max_results, bool nsh_b_is_lan)
{
	if (nsh_b_operation_in_progress)
	{
		BroadcastFailure(TEXT("FindRooms"), TEXT("Another session operation is already in progress."));
		return;
	}

	if (!EnsureSessionInterface(TEXT("FindRooms")))
	{
		nsh_b_quick_match_pending = false;
		return;
	}

	nsh_b_operation_in_progress = true;
	nsh_cached_rooms.Reset();
	nsh_session_search = MakeShared<FOnlineSessionSearch>();
	nsh_session_search->MaxSearchResults = FMath::Clamp(nsh_max_results, 1, 200);
	nsh_session_search->bIsLanQuery = nsh_b_is_lan;
	nsh_session_search->PingBucketSize = 50;
	nsh_session_search->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	nsh_session_search->QuerySettings.Set(nsh_game_key, FString(TEXT("FPSJANGGI_V1")), EOnlineComparisonOp::Equals);

	if (!nsh_session_interface->FindSessions(0, nsh_session_search.ToSharedRef()))
	{
		ResetOperationState();
		BroadcastFailure(TEXT("FindRooms"), TEXT("The room search could not be started."));
	}
}

void USessionSubsystem::JoinRoom(int32 nsh_room_index)
{
	if (nsh_b_operation_in_progress)
	{
		BroadcastFailure(TEXT("JoinRoom"), TEXT("Another session operation is already in progress."));
		return;
	}

	if (!EnsureSessionInterface(TEXT("JoinRoom")))
	{
		return;
	}

	if (!nsh_session_search.IsValid() || !nsh_session_search->SearchResults.IsValidIndex(nsh_room_index))
	{
		BroadcastFailure(TEXT("JoinRoom"), TEXT("The selected room is no longer available. Search again."));
		return;
	}

	nsh_b_operation_in_progress = true;
	nsh_pending_join_index = nsh_room_index;

	if (nsh_session_interface->GetNamedSession(nsh_session_name) != nullptr)
	{
		nsh_b_join_after_destroy = true;
		if (!nsh_session_interface->DestroySession(nsh_session_name))
		{
			ResetOperationState();
			BroadcastFailure(TEXT("JoinRoom"), TEXT("Could not leave the current room."));
		}
		return;
	}

	BeginJoinRoom(nsh_room_index);
}

void USessionSubsystem::QuickMatch(int32 nsh_max_players, bool nsh_b_is_lan)
{
	if (nsh_b_operation_in_progress)
	{
		BroadcastFailure(TEXT("QuickMatch"), TEXT("Another session operation is already in progress."));
		return;
	}

	nsh_b_quick_match_pending = true;
	nsh_pending_max_players = FMath::Clamp(nsh_max_players, 2, 8);
	nsh_b_pending_is_lan = nsh_b_is_lan;
	nsh_pending_room_name = TEXT("Quick Match");
	FindRooms(50, nsh_b_is_lan);
}

void USessionSubsystem::LeaveRoom()
{
	if (nsh_b_operation_in_progress)
	{
		BroadcastFailure(TEXT("LeaveRoom"), TEXT("Another session operation is already in progress."));
		return;
	}

	if (!EnsureSessionInterface(TEXT("LeaveRoom")))
	{
		return;
	}

	if (nsh_session_interface->GetNamedSession(nsh_session_name) == nullptr)
	{
		nsh_on_session_operation_complete.Broadcast(TEXT("LeaveRoom"), true, TEXT("There is no active room."));
		return;
	}

	nsh_b_operation_in_progress = true;
	if (!nsh_session_interface->DestroySession(nsh_session_name))
	{
		ResetOperationState();
		BroadcastFailure(TEXT("LeaveRoom"), TEXT("The room could not be closed."));
	}
}

bool USessionSubsystem::EnsureSessionInterface(FName nsh_operation)
{
	if (!nsh_session_interface.IsValid())
	{
		if (IOnlineSubsystem* nsh_online_subsystem = IOnlineSubsystem::Get())
		{
			nsh_session_interface = nsh_online_subsystem->GetSessionInterface();
		}
	}

	if (!nsh_session_interface.IsValid())
	{
		BroadcastFailure(nsh_operation, TEXT("The online subsystem is unavailable. Start Steam before launching the game."));
		return false;
	}

	BindSessionDelegates();
	return true;
}

void USessionSubsystem::BindSessionDelegates()
{
	if (!nsh_session_interface.IsValid() || nsh_b_delegates_bound)
	{
		return;
	}

	nsh_create_delegate_handle = nsh_session_interface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &USessionSubsystem::HandleCreateSessionComplete));
	nsh_find_delegate_handle = nsh_session_interface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &USessionSubsystem::HandleFindSessionsComplete));
	nsh_join_delegate_handle = nsh_session_interface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &USessionSubsystem::HandleJoinSessionComplete));
	nsh_destroy_delegate_handle = nsh_session_interface->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &USessionSubsystem::HandleDestroySessionComplete));
	nsh_b_delegates_bound = true;
}

void USessionSubsystem::BeginCreateRoom()
{
	FOnlineSessionSettings nsh_settings;
	nsh_settings.bIsLANMatch = nsh_b_pending_is_lan;
	nsh_settings.NumPublicConnections = nsh_pending_max_players;
	nsh_settings.NumPrivateConnections = 0;
	nsh_settings.bShouldAdvertise = true;
	nsh_settings.bAllowJoinInProgress = true;
	nsh_settings.bAllowInvites = true;
	nsh_settings.bUsesPresence = true;
	nsh_settings.bAllowJoinViaPresence = true;
	nsh_settings.bUseLobbiesIfAvailable = true;
	nsh_settings.bUseLobbiesVoiceChatIfAvailable = false;
	nsh_settings.Set(nsh_room_name_key, nsh_pending_room_name, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	nsh_settings.Set(nsh_game_key, FString(TEXT("FPSJANGGI_V1")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	nsh_settings.Set(SETTING_MAPNAME, FString(TEXT("MainMap")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	nsh_settings.Set(SETTING_GAMEMODE, FString(TEXT("FPSJanggi")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	if (!nsh_session_interface->CreateSession(0, nsh_session_name, nsh_settings))
	{
		ResetOperationState();
		BroadcastFailure(TEXT("CreateRoom"), TEXT("The room creation request could not be started."));
	}
}

void USessionSubsystem::BeginJoinRoom(int32 nsh_room_index)
{
	if (!nsh_session_search.IsValid() || !nsh_session_search->SearchResults.IsValidIndex(nsh_room_index))
	{
		ResetOperationState();
		BroadcastFailure(TEXT("JoinRoom"), TEXT("The selected room expired. Search again."));
		return;
	}

	if (!nsh_session_interface->JoinSession(0, nsh_session_name, nsh_session_search->SearchResults[nsh_room_index]))
	{
		ResetOperationState();
		BroadcastFailure(TEXT("JoinRoom"), TEXT("The join request could not be started."));
	}
}

void USessionSubsystem::BroadcastFailure(FName nsh_operation, const FString& nsh_message)
{
	nsh_on_session_operation_complete.Broadcast(nsh_operation, false, nsh_message);
}

void USessionSubsystem::ResetOperationState()
{
	nsh_b_operation_in_progress = false;
	nsh_b_quick_match_pending = false;
	nsh_b_create_after_destroy = false;
	nsh_b_join_after_destroy = false;
	nsh_pending_join_index = INDEX_NONE;
}

void USessionSubsystem::HandleCreateSessionComplete(FName nsh_completed_session_name, bool nsh_b_was_successful)
{
	if (nsh_completed_session_name != nsh_session_name)
	{
		return;
	}

	nsh_b_operation_in_progress = false;
	nsh_on_session_operation_complete.Broadcast(TEXT("CreateRoom"), nsh_b_was_successful,
		nsh_b_was_successful ? TEXT("Room created.") : TEXT("Steam could not create the room."));

	if (nsh_b_was_successful)
	{
		if (UWorld* nsh_world = GetWorld())
		{
			nsh_world->ServerTravel(TEXT("/Game/User/Map/MainMap?listen"));
		}
	}
}

void USessionSubsystem::HandleFindSessionsComplete(bool nsh_b_was_successful)
{
	nsh_b_operation_in_progress = false;
	nsh_cached_rooms.Reset();

	if (nsh_b_was_successful && nsh_session_search.IsValid())
	{
		for (int32 nsh_index = 0; nsh_index < nsh_session_search->SearchResults.Num(); ++nsh_index)
		{
			const FOnlineSessionSearchResult& nsh_result = nsh_session_search->SearchResults[nsh_index];
			const FOnlineSession& nsh_session = nsh_result.Session;

			FNSHRoomInfo nsh_room;
			nsh_room.nsh_room_index = nsh_index;
			nsh_room.nsh_host_name = nsh_session.OwningUserName;
			nsh_room.nsh_session_id = nsh_session.GetSessionIdStr();
			nsh_room.nsh_max_players = nsh_session.SessionSettings.NumPublicConnections;
			nsh_room.nsh_current_players = FMath::Max(0, nsh_room.nsh_max_players - nsh_session.NumOpenPublicConnections);
			nsh_room.nsh_ping_ms = nsh_result.PingInMs;
			if (!nsh_session.SessionSettings.Get(nsh_room_name_key, nsh_room.nsh_room_name))
			{
				nsh_room.nsh_room_name = nsh_room.nsh_host_name.IsEmpty() ? TEXT("FPS Janggi Room") : nsh_room.nsh_host_name;
			}
			nsh_cached_rooms.Add(MoveTemp(nsh_room));
		}
	}

	nsh_on_room_search_complete.Broadcast(nsh_b_was_successful, nsh_cached_rooms);
	nsh_on_session_operation_complete.Broadcast(TEXT("FindRooms"), nsh_b_was_successful,
		nsh_b_was_successful ? FString::Printf(TEXT("Found %d room(s)."), nsh_cached_rooms.Num()) : TEXT("Room search failed."));

	if (!nsh_b_quick_match_pending)
	{
		return;
	}

	nsh_b_quick_match_pending = false;
	int32 nsh_joinable_index = INDEX_NONE;
	for (const FNSHRoomInfo& nsh_room : nsh_cached_rooms)
	{
		if (nsh_room.nsh_current_players < nsh_room.nsh_max_players)
		{
			nsh_joinable_index = nsh_room.nsh_room_index;
			break;
		}
	}

	if (nsh_joinable_index != INDEX_NONE)
	{
		JoinRoom(nsh_joinable_index);
	}
	else
	{
		CreateRoom(nsh_pending_room_name, nsh_pending_max_players, nsh_b_pending_is_lan);
	}
}

void USessionSubsystem::HandleJoinSessionComplete(FName nsh_completed_session_name, EOnJoinSessionCompleteResult::Type nsh_result)
{
	if (nsh_completed_session_name != nsh_session_name)
	{
		return;
	}

	nsh_b_operation_in_progress = false;
	const bool nsh_b_joined = nsh_result == EOnJoinSessionCompleteResult::Success;
	FString nsh_connect_string;
	const bool nsh_b_has_address = nsh_b_joined && nsh_session_interface.IsValid()
		&& nsh_session_interface->GetResolvedConnectString(nsh_session_name, nsh_connect_string);

	if (nsh_b_has_address)
	{
		if (UGameInstance* nsh_game_instance = GetGameInstance())
		{
			if (APlayerController* nsh_player_controller = nsh_game_instance->GetFirstLocalPlayerController())
			{
				nsh_player_controller->ClientTravel(nsh_connect_string, TRAVEL_Absolute);
			}
		}
	}

	const bool nsh_b_success = nsh_b_joined && nsh_b_has_address;
	nsh_on_session_operation_complete.Broadcast(TEXT("JoinRoom"), nsh_b_success,
		nsh_b_success ? TEXT("Joined room.") : TEXT("The room could not be joined or its address was unavailable."));
}

void USessionSubsystem::HandleDestroySessionComplete(FName nsh_completed_session_name, bool nsh_b_was_successful)
{
	if (nsh_completed_session_name != nsh_session_name)
	{
		return;
	}

	const bool nsh_b_should_create = nsh_b_create_after_destroy;
	const bool nsh_b_should_join = nsh_b_join_after_destroy;
	const int32 nsh_join_index = nsh_pending_join_index;
	nsh_b_create_after_destroy = false;
	nsh_b_join_after_destroy = false;
	nsh_pending_join_index = INDEX_NONE;

	if (!nsh_b_was_successful)
	{
		nsh_b_operation_in_progress = false;
		BroadcastFailure(nsh_b_should_create ? TEXT("CreateRoom") : nsh_b_should_join ? TEXT("JoinRoom") : TEXT("LeaveRoom"),
			TEXT("The existing room could not be closed."));
		return;
	}

	if (nsh_b_should_create)
	{
		BeginCreateRoom();
		return;
	}

	if (nsh_b_should_join)
	{
		BeginJoinRoom(nsh_join_index);
		return;
	}

	nsh_b_operation_in_progress = false;
	nsh_on_session_operation_complete.Broadcast(TEXT("LeaveRoom"), true, TEXT("Left room."));
}
