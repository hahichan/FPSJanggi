import unreal


kkw_character_roots = [
    "/Game/User/Charactor/Cannon_R",
    "/Game/User/Charactor/Cannon_B",
    "/Game/User/Charactor/Guard_R",
    "/Game/User/Charactor/Guard_B",
    "/Game/User/Charactor/Chariot_R",
    "/Game/User/Charactor/Chariot_B",
]

kkw_registry = unreal.AssetRegistryHelpers.get_asset_registry()

for kkw_root in kkw_character_roots:
    kkw_assets = kkw_registry.get_assets_by_path(kkw_root, recursive=True)
    unreal.log("[KKWAnimInspect] root={}".format(kkw_root))
    for kkw_asset_data in sorted(kkw_assets, key=lambda kkw_item: kkw_item.asset_name):
        kkw_class_name = str(kkw_asset_data.asset_class_path.asset_name)
        if kkw_class_name in ("AnimSequence", "SkeletalMesh", "Skeleton"):
            unreal.log(
                "[KKWAnimInspect] class={klass} path={path}".format(
                    klass=kkw_class_name,
                    path=kkw_asset_data.package_name,
                )
            )
