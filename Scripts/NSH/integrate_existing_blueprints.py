"""Connect the existing board Blueprints to the standalone NSH base classes.

The script is idempotent. It preserves every existing Blueprint graph and only
changes the parent classes when they do not already match.
"""

import unreal


INTEGRATIONS = (
    (
        "/Game/User/Blueprints/JanggiBoard1.JanggiBoard1",
        "/Script/FPSJanggi.AuthoritativeJanggiBoard",
    ),
    (
        "/Game/User/Blueprints/NewPlayerController.NewPlayerController",
        "/Script/FPSJanggi.BoardPlayerController",
    ),
    (
        "/Game/User/Blueprints/NewGameMode.NewGameMode",
        "/Script/FPSJanggi.BoardGameMode",
    ),
)


def integrate(asset_path, parent_class_path):
    blueprint = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not isinstance(blueprint, unreal.Blueprint):
        raise RuntimeError(f"Blueprint not found: {asset_path}")

    parent_class = unreal.load_class(None, parent_class_path)
    if parent_class is None:
        raise RuntimeError(f"C++ parent class not found: {parent_class_path}")

    current_parent = unreal.BlueprintEditorLibrary.get_blueprint_parent_class(blueprint)
    current_parent_path = current_parent.get_path_name() if current_parent else "None"
    if current_parent_path != parent_class_path:
        unreal.log(f"NSH_INTEGRATE: {asset_path}: {current_parent_path} -> {parent_class_path}")
        unreal.BlueprintEditorLibrary.reparent_blueprint(blueprint, parent_class)
    else:
        unreal.log(f"NSH_INTEGRATE: {asset_path}: already integrated")

    if not unreal.BlueprintEditorLibrary.compile_blueprint(blueprint):
        raise RuntimeError(f"Blueprint compile failed after integration: {asset_path}")
    if not unreal.EditorAssetLibrary.save_loaded_asset(blueprint, only_if_is_dirty=False):
        raise RuntimeError(f"Blueprint save failed after integration: {asset_path}")


def main():
    for asset_path, parent_class_path in INTEGRATIONS:
        integrate(asset_path, parent_class_path)
    unreal.log("NSH_INTEGRATION_COMPLETE")


if __name__ == "__main__":
    main()
