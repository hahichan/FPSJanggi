import unreal


kkw_map_path = "/Game/User/Map/MainMap"

unreal.EditorLoadingAndSavingUtils.load_map(kkw_map_path)
kkw_world = unreal.EditorLevelLibrary.get_editor_world()

kkw_actors = unreal.GameplayStatics.get_all_actors_of_class(kkw_world, unreal.Actor)
kkw_skeletal_like = []

for kkw_actor in kkw_actors:
    kkw_label = kkw_actor.get_actor_label()
    kkw_name = kkw_actor.get_name()
    kkw_components = kkw_actor.get_components_by_class(unreal.SkeletalMeshComponent)
    kkw_class_name = kkw_actor.get_class().get_path_name()
    if kkw_components or "skeletal" in kkw_label.lower() or "skeleton" in kkw_label.lower() or "janggi" in kkw_label.lower():
        kkw_component = kkw_components[0] if kkw_components else None
        kkw_mesh = kkw_component.skeletal_mesh if kkw_component else None
        kkw_actor_rotation = kkw_actor.get_actor_rotation()
        kkw_component_rotation = kkw_component.get_world_rotation() if kkw_component else None
        kkw_bounds = kkw_mesh.get_bounds() if kkw_mesh else None
        kkw_skeletal_like.append((
            kkw_label,
            kkw_name,
            kkw_class_name,
            kkw_actor.get_actor_location(),
            kkw_actor_rotation,
            kkw_component_rotation,
            kkw_mesh.get_path_name() if kkw_mesh else "None",
            kkw_bounds.origin if kkw_bounds else "None",
            kkw_bounds.box_extent if kkw_bounds else "None",
        ))

unreal.log("[FPSJanggiInspect] Actor count: {}".format(len(kkw_actors)))
unreal.log("[FPSJanggiInspect] Skeletal/name-like count: {}".format(len(kkw_skeletal_like)))

for kkw_label, kkw_name, kkw_class_name, kkw_location, kkw_actor_rotation, kkw_component_rotation, kkw_mesh, kkw_bounds_origin, kkw_bounds_extent in sorted(kkw_skeletal_like, key=lambda kkw_item: kkw_item[0]):
    unreal.log(
        "[FPSJanggiInspect] label={kkw_label} name={kkw_name} class={kkw_class} location={kkw_location} actor_rotation={kkw_actor_rotation} component_rotation={kkw_component_rotation} mesh={kkw_mesh} bounds_origin={kkw_bounds_origin} bounds_extent={kkw_bounds_extent}".format(
            kkw_label=kkw_label,
            kkw_name=kkw_name,
            kkw_class=kkw_class_name,
            kkw_location=kkw_location,
            kkw_actor_rotation=kkw_actor_rotation,
            kkw_component_rotation=kkw_component_rotation,
            kkw_mesh=kkw_mesh,
            kkw_bounds_origin=kkw_bounds_origin,
            kkw_bounds_extent=kkw_bounds_extent,
        )
    )
