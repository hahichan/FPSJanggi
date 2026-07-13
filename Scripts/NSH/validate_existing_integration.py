"""Read-only smoke validation for the board/C++ integration."""

import unreal


GAME_MODE_BP = "/Game/User/Blueprints/NewGameMode.NewGameMode"
PLAYER_CONTROLLER_BP = "/Game/User/Blueprints/NewPlayerController.NewPlayerController"
BOARD_BP = "/Game/User/Blueprints/JanggiBoard1.JanggiBoard1"
MAIN_MAP = "/Game/User/Map/MainMap"


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def parent_path(asset_path):
    blueprint = unreal.EditorAssetLibrary.load_asset(asset_path)
    require(isinstance(blueprint, unreal.Blueprint), f"Missing Blueprint: {asset_path}")
    parent = unreal.BlueprintEditorLibrary.get_blueprint_parent_class(blueprint)
    require(parent is not None, f"Missing parent class: {asset_path}")
    return parent.get_path_name()


def main():
    require(
        parent_path(BOARD_BP) == "/Script/FPSJanggi.AuthoritativeJanggiBoard",
        "JanggiBoard is not connected to the authoritative C++ board",
    )
    require(
        parent_path(PLAYER_CONTROLLER_BP)
        == "/Script/FPSJanggi.BoardPlayerController",
        "NewPlayerController is not connected to the NSH C++ controller",
    )
    require(
        parent_path(GAME_MODE_BP) == "/Script/FPSJanggi.BoardGameMode",
        "NewGameMode is not connected to the NSH C++ game mode",
    )

    game_mode_class = unreal.load_class(None, GAME_MODE_BP + "_C")
    controller_class = unreal.load_class(None, PLAYER_CONTROLLER_BP + "_C")
    session_class = unreal.load_class(None, "/Script/FPSJanggi.SessionSubsystem")
    require(game_mode_class is not None, "Integrated game mode class did not load")
    require(controller_class is not None, "Integrated player controller class did not load")
    require(session_class is not None, "NSH session subsystem class did not load")

    game_mode_default = unreal.get_default_object(game_mode_class)
    configured_controller = game_mode_default.get_editor_property("player_controller_class")
    configured_pawn = game_mode_default.get_editor_property("default_pawn_class")
    require(
        configured_controller.get_path_name() == PLAYER_CONTROLLER_BP + "_C",
        "NewGameMode no longer uses NewPlayerController",
    )
    require(configured_pawn is not None, "Existing board DefaultPawn was lost")

    unreal.EditorLoadingAndSavingUtils.load_map(MAIN_MAP)
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    boards = [
        actor
        for actor in actor_subsystem.get_all_level_actors()
        if actor.get_class().get_path_name() == BOARD_BP + "_C"
    ]
    require(len(boards) == 1, f"Expected exactly one existing JanggiBoard1 in MainMap, found {len(boards)}")
    require(boards[0].get_editor_property("replicates"), "JanggiBoard actor replication is disabled")

    unreal.log("NSH_INTEGRATION_VALID: board + standalone NSH controller/game mode + session subsystem")


if __name__ == "__main__":
    main()
