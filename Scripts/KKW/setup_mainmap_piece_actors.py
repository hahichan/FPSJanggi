import unreal


kkw_map_path = "/Game/User/Map/MainMap"

kkw_pieces = [
    ("SkeletalMeshActor", "/Game/User/Charactor/Cannon_R/SK_Cannon_R_CharacterWeapon.SK_Cannon_R_CharacterWeapon", (-420.0, -260.0, 170.0), 0.0),
    ("SkeletalMeshActor2", "/Game/User/Charactor/Cannon_B/SK_Cannon_B_CharacterWeapon.SK_Cannon_B_CharacterWeapon", (-420.0, 260.0, 170.0), 180.0),
    ("SkeletalMeshActor3", "/Game/User/Charactor/Guard_R/SK_Guard_R_CharacterWeapon.SK_Guard_R_CharacterWeapon", (0.0, -260.0, 170.0), 0.0),
    ("SkeletalMeshActor4", "/Game/User/Charactor/Guard_B/SK_Guard_B_CharacterWeapon.SK_Guard_B_CharacterWeapon", (0.0, 260.0, 170.0), 180.0),
    ("SkeletalMeshActor5", "/Game/User/Charactor/Chariot_R/SK_Chariot_R_CharacterWeapon.SK_Chariot_R_CharacterWeapon", (420.0, -260.0, 170.0), 0.0),
    ("SkeletalMeshActor6", "/Game/User/Charactor/Chariot_B/SK_Chariot_B_CharacterWeapon.SK_Chariot_B_CharacterWeapon", (420.0, 260.0, 170.0), 180.0),
]


def kkw_normalized(kkw_value):
    return kkw_value.lower().replace(" ", "").replace("_", "").replace("-", "")


unreal.EditorLoadingAndSavingUtils.load_map(kkw_map_path)
kkw_world = unreal.EditorLevelLibrary.get_editor_world()
kkw_all_actors = unreal.GameplayStatics.get_all_actors_of_class(kkw_world, unreal.Actor)
kkw_piece_labels = {kkw_normalized(kkw_piece[0]) for kkw_piece in kkw_pieces}

kkw_center = unreal.Vector(0.0, 0.0, 0.0)
for kkw_actor in kkw_all_actors:
    if "janggiboard" in kkw_normalized(kkw_actor.get_actor_label()):
        kkw_center = kkw_actor.get_actor_location()
        break

for kkw_actor in kkw_all_actors:
    if kkw_normalized(kkw_actor.get_actor_label()) in kkw_piece_labels:
        unreal.EditorLevelLibrary.destroy_actor(kkw_actor)

for kkw_label, kkw_mesh_path, kkw_offset, kkw_yaw in kkw_pieces:
    kkw_mesh = unreal.load_asset(kkw_mesh_path)
    if kkw_mesh is None:
        raise RuntimeError("Missing mesh: " + kkw_mesh_path)

    kkw_location = kkw_center + unreal.Vector(*kkw_offset)
    kkw_rotation = unreal.Rotator(0.0, kkw_yaw, 0.0)
    kkw_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkeletalMeshActor, kkw_location, kkw_rotation)
    kkw_actor.set_actor_label(kkw_label, mark_dirty=True)
    kkw_actor.set_editor_property("tags", [kkw_label])
    kkw_component = kkw_actor.get_component_by_class(unreal.SkeletalMeshComponent)
    kkw_component.set_editor_property("skeletal_mesh", kkw_mesh)
    kkw_component.set_editor_property("animation_mode", unreal.AnimationMode.ANIMATION_SINGLE_NODE)
    kkw_component.set_world_scale3d(unreal.Vector(1.0, 1.0, 1.0))
    unreal.log("[PieceSetup] placed {} -> {}".format(kkw_label, kkw_mesh_path))

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log("[PieceSetup] MainMap saved")
