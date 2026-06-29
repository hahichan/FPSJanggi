import unreal


kkw_required_classes = [
    "/Script/FPSJanggi.FPSJanggiAbilityGameMode",
    "/Script/FPSJanggi.FPSJanggiAbilityPlayerController",
    "/Script/FPSJanggi.FPSJanggiAbilityCharacter",
    "/Script/FPSJanggi.FPSJanggiAbilityProjectile",
]

kkw_required_assets = [
    "/Game/User/Map/MainMap.MainMap",
    "/Game/User/Charactor/Cannon_R/SK_Cannon_R_CharacterWeapon.SK_Cannon_R_CharacterWeapon",
    "/Game/User/Charactor/Cannon_B/SK_Cannon_B_CharacterWeapon.SK_Cannon_B_CharacterWeapon",
    "/Game/User/Charactor/Guard_R/SK_Guard_R_CharacterWeapon.SK_Guard_R_CharacterWeapon",
    "/Game/User/Charactor/Guard_B/SK_Guard_B_CharacterWeapon.SK_Guard_B_CharacterWeapon",
    "/Game/User/Charactor/Chariot_R/SK_Chariot_R_CharacterWeapon.SK_Chariot_R_CharacterWeapon",
    "/Game/User/Charactor/Chariot_B/SK_Chariot_B_CharacterWeapon.SK_Chariot_B_CharacterWeapon",
    "/Game/User/Charactor/Cannon_R/SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_Idle_canon.SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_Idle_canon",
    "/Game/User/Charactor/Cannon_R/SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_Move_canon.SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_Move_canon",
    "/Game/User/Charactor/Cannon_R/SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_attack_canon.SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_attack_canon",
    "/Game/User/Charactor/Cannon_R/SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_run_canon.SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_run_canon",
    "/Game/User/Charactor/Cannon_B/SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_Idle_canon.SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_Idle_canon",
    "/Game/User/Charactor/Cannon_B/SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_Move_canon.SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_Move_canon",
    "/Game/User/Charactor/Cannon_B/SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_attack_canon.SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_attack_canon",
    "/Game/User/Charactor/Cannon_B/SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_run_canon.SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_run_canon",
    "/Game/User/Charactor/Guard_R/SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_Idle.SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_Idle",
    "/Game/User/Charactor/Guard_R/SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_Move.SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_Move",
    "/Game/User/Charactor/Guard_R/SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_attack_sheid.SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_attack_sheid",
    "/Game/User/Charactor/Guard_R/SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_run.SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_run",
    "/Game/User/Charactor/Guard_B/SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_Idle.SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_Idle",
    "/Game/User/Charactor/Guard_B/SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_Move.SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_Move",
    "/Game/User/Charactor/Guard_B/SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_attack_sheid.SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_attack_sheid",
    "/Game/User/Charactor/Guard_B/SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_run.SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_run",
    "/Game/User/Charactor/Chariot_R/SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_Idle.SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_Idle",
    "/Game/User/Charactor/Chariot_R/SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_Move.SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_Move",
    "/Game/User/Charactor/Chariot_R/SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_attack_ammor.SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_attack_ammor",
    "/Game/User/Charactor/Chariot_R/SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_run.SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_run",
    "/Game/User/Charactor/Chariot_B/SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_Idle.SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_Idle",
    "/Game/User/Charactor/Chariot_B/SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_Move.SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_Move",
    "/Game/User/Charactor/Chariot_B/SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_attack_ammor.SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_attack_ammor",
    "/Game/User/Charactor/Chariot_B/SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_run.SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_run",
    "/Engine/EngineSounds/1kSineTonePing.1kSineTonePing",
]


def kkw_require(kkw_condition, kkw_message):
    if not kkw_condition:
        raise RuntimeError(kkw_message)


for kkw_class_path in kkw_required_classes:
    kkw_loaded_class = unreal.load_class(None, kkw_class_path)
    kkw_require(kkw_loaded_class is not None, "Missing class: " + kkw_class_path)
    unreal.log("[AbilitySetup] class ok: " + kkw_class_path)

for kkw_asset_path in kkw_required_assets:
    kkw_loaded_asset = unreal.load_asset(kkw_asset_path)
    kkw_require(kkw_loaded_asset is not None, "Missing asset: " + kkw_asset_path)
    unreal.log("[AbilitySetup] asset ok: " + kkw_asset_path)

unreal.EditorLoadingAndSavingUtils.load_map("/Game/User/Map/MainMap")
unreal.log("[AbilitySetup] validation complete")
