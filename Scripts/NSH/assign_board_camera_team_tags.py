"""Assign MainMap's two existing board cameras to the side they face from.

Camera locations and rotations are preserved. Only the semantic Blue/Red tags
are replaced so each local player sees their own pieces on the near side.
"""

import unreal


MAIN_MAP = "/Game/User/Map/MainMap"
CAMERA_TEAM_BY_NAME = {
    "CameraActor_1": "Red",
    "CameraActor_2": "Blue",
}


def main():
    world = unreal.EditorLoadingAndSavingUtils.load_map(MAIN_MAP)
    if world is None:
        raise RuntimeError(f"Failed to load map: {MAIN_MAP}")
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    updated = 0

    for actor in actors:
        if not isinstance(actor, unreal.CameraActor):
            continue

        team = CAMERA_TEAM_BY_NAME.get(actor.get_name())
        if team is None:
            continue

        preserved_tags = [tag for tag in actor.tags if str(tag) not in ("Blue", "Red")]
        actor.tags = preserved_tags + [team]
        unreal.log(
            "BOARD_CAMERA_TAG_ASSIGNED "
            f"name={actor.get_name()} team={team} "
            f"location={actor.get_actor_location()} rotation={actor.get_actor_rotation()}"
        )
        updated += 1

    if updated != len(CAMERA_TEAM_BY_NAME):
        raise RuntimeError(
            f"Expected {len(CAMERA_TEAM_BY_NAME)} board cameras, updated {updated}"
        )

    if not unreal.EditorLoadingAndSavingUtils.save_map(world, MAIN_MAP):
        raise RuntimeError(f"Failed to save map: {MAIN_MAP}")

    unreal.log("BOARD_CAMERA_TAG_ASSIGNMENT_COMPLETE")


if __name__ == "__main__":
    main()
