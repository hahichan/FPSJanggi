"""Read-only diagnostic for placed MainMap board cameras."""

import unreal


MAIN_MAP = "/Game/User/Map/MainMap"


def safe_property(obj, name):
    try:
        return obj.get_editor_property(name)
    except Exception:
        return "<unavailable>"


def main():
    unreal.EditorLoadingAndSavingUtils.load_map(MAIN_MAP)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    for actor in actors:
        if isinstance(actor, unreal.CameraActor):
            component = safe_property(actor, "camera_component")
            unreal.log(
                "BOARD_CAMERA_ASSET "
                f"name={actor.get_name()} "
                f"label={actor.get_actor_label()} "
                f"location={actor.get_actor_location()} "
                f"rotation={actor.get_actor_rotation()} "
                f"tags={list(actor.tags)} "
                f"auto_player={safe_property(actor, 'auto_activate_for_player')} "
                f"fov={safe_property(component, 'field_of_view') if component != '<unavailable>' else '<unavailable>'}"
            )
    unreal.log("BOARD_CAMERA_INSPECTION_COMPLETE")


if __name__ == "__main__":
    main()
