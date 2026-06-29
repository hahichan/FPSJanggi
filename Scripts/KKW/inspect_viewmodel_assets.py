import unreal


kkw_mesh_paths = [
    "/Game/User/Charactor/Cannon_R/SK_Cannon_R_CharacterWeapon",
    "/Game/User/Charactor/Cannon_B/SK_Cannon_B_CharacterWeapon",
    "/Game/User/Charactor/Guard_R/SK_Guard_R_CharacterWeapon",
    "/Game/User/Charactor/Guard_B/SK_Guard_B_CharacterWeapon",
    "/Game/User/Charactor/Chariot_R/SK_Chariot_R_CharacterWeapon",
    "/Game/User/Charactor/Chariot_B/SK_Chariot_B_CharacterWeapon",
]

kkw_map_path = "/Game/User/Map/MainMap"

unreal.EditorLoadingAndSavingUtils.load_map(kkw_map_path)
kkw_world = unreal.EditorLevelLibrary.get_editor_world()

unreal.log("[FPSJanggiViewmodelInspect] Skeletal meshes")
for kkw_mesh_path in kkw_mesh_paths:
    kkw_mesh = unreal.load_asset(kkw_mesh_path)
    if not kkw_mesh:
        unreal.log_error("[FPSJanggiViewmodelInspect] missing mesh {}".format(kkw_mesh_path))
        continue

    kkw_bounds = kkw_mesh.get_bounds()
    unreal.log(
        "[FPSJanggiViewmodelInspect] mesh={} bounds_origin={} bounds_extent={}".format(
            kkw_mesh.get_path_name(),
            kkw_bounds.origin,
            kkw_bounds.box_extent,
        )
    )

    try:
        kkw_materials = kkw_mesh.get_editor_property("materials")
    except Exception as kkw_error:
        unreal.log_warning("[FPSJanggiViewmodelInspect] material read failed {} {}".format(kkw_mesh_path, kkw_error))
        kkw_materials = []

    for kkw_index, kkw_material in enumerate(kkw_materials):
        kkw_slot_name = getattr(kkw_material, "material_slot_name", "None")
        kkw_material_interface = getattr(kkw_material, "material_interface", None)
        kkw_material_path = kkw_material_interface.get_path_name() if kkw_material_interface else "None"
        unreal.log(
            "[FPSJanggiViewmodelInspect] material mesh={} index={} slot={} asset={}".format(
                kkw_mesh_path,
                kkw_index,
                kkw_slot_name,
                kkw_material_path,
            )
        )

unreal.log("[FPSJanggiViewmodelInspect] Source actors")
for kkw_actor in unreal.GameplayStatics.get_all_actors_of_class(kkw_world, unreal.Actor):
    kkw_label = kkw_actor.get_actor_label()
    if not kkw_label.startswith("SkeletalMeshActor"):
        continue

    kkw_component = kkw_actor.get_component_by_class(unreal.SkeletalMeshComponent)
    kkw_mesh = kkw_component.skeletal_mesh if kkw_component else None
    unreal.log(
        "[FPSJanggiViewmodelInspect] actor={} actor_scale={} component_scale={} mesh={}".format(
            kkw_label,
            kkw_actor.get_actor_scale3d(),
            kkw_component.get_world_scale() if kkw_component else "None",
            kkw_mesh.get_path_name() if kkw_mesh else "None",
        )
    )
