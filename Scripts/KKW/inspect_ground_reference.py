import unreal


kkw_map_path = "/Game/User/Map/MainMap"
kkw_match_terms = [
    "wood_medieval_defense61",
    "wood_medieval_defense",
    "skeletalmeshactor",
    "landscape",
    "stone",
]


def kkw_norm(kkw_value):
    return str(kkw_value).lower().replace(" ", "").replace("_", "").replace("-", "")


def kkw_vec_text(kkw_vector):
    return "({:.3f},{:.3f},{:.3f})".format(kkw_vector.x, kkw_vector.y, kkw_vector.z)


def kkw_actor_matches(kkw_actor):
    kkw_label = kkw_norm(kkw_actor.get_actor_label())
    kkw_name = kkw_norm(kkw_actor.get_name())
    return any(kkw_norm(kkw_term) in kkw_label or kkw_norm(kkw_term) in kkw_name for kkw_term in kkw_match_terms)


unreal.EditorLoadingAndSavingUtils.load_map(kkw_map_path)
kkw_world = unreal.EditorLevelLibrary.get_editor_world()
kkw_actors = unreal.GameplayStatics.get_all_actors_of_class(kkw_world, unreal.Actor)

unreal.log("[FPSJanggiGroundInspect] Actor count: {}".format(len(kkw_actors)))

for kkw_actor in sorted([kkw_actor for kkw_actor in kkw_actors if kkw_actor_matches(kkw_actor)], key=lambda kkw_item: kkw_item.get_actor_label()):
    kkw_origin = unreal.Vector()
    kkw_extent = unreal.Vector()
    try:
        kkw_origin, kkw_extent = kkw_actor.get_actor_bounds(False)
    except Exception as kkw_error:
        unreal.log_warning("[FPSJanggiGroundInspect] bounds failed label={} error={}".format(kkw_actor.get_actor_label(), kkw_error))

    kkw_static_component = kkw_actor.get_component_by_class(unreal.StaticMeshComponent)
    kkw_skeletal_component = kkw_actor.get_component_by_class(unreal.SkeletalMeshComponent)
    kkw_mesh_path = "None"
    kkw_component_scale = "None"

    if kkw_static_component:
        kkw_mesh = kkw_static_component.static_mesh
        kkw_mesh_path = kkw_mesh.get_path_name() if kkw_mesh else "None"
        kkw_component_scale = kkw_vec_text(kkw_static_component.get_world_scale())
    elif kkw_skeletal_component:
        kkw_mesh = kkw_skeletal_component.skeletal_mesh
        kkw_mesh_path = kkw_mesh.get_path_name() if kkw_mesh else "None"
        kkw_component_scale = kkw_vec_text(kkw_skeletal_component.get_world_scale())

    kkw_class_name = kkw_actor.get_class().get_path_name()
    kkw_label = kkw_actor.get_actor_label()
    kkw_location = kkw_actor.get_actor_location()
    kkw_rotation = kkw_actor.get_actor_rotation()
    unreal.log(
        "[FPSJanggiGroundInspect] label={label} name={name} class={cls} loc={loc} rot=({pitch:.3f},{yaw:.3f},{roll:.3f}) actor_scale={actor_scale} component_scale={component_scale} bounds_origin={origin} bounds_extent={extent} mesh={mesh}".format(
            label=kkw_label,
            name=kkw_actor.get_name(),
            cls=kkw_class_name,
            loc=kkw_vec_text(kkw_location),
            pitch=kkw_rotation.pitch,
            yaw=kkw_rotation.yaw,
            roll=kkw_rotation.roll,
            actor_scale=kkw_vec_text(kkw_actor.get_actor_scale3d()),
            component_scale=kkw_component_scale,
            origin=kkw_vec_text(kkw_origin),
            extent=kkw_vec_text(kkw_extent),
            mesh=kkw_mesh_path,
        )
    )
