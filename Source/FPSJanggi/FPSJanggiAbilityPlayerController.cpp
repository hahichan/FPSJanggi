// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSJanggiAbilityPlayerController.h"

#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "KKW/KKWProjectScriptsActor.h"
#include "Kismet/GameplayStatics.h"

namespace
{
struct FPieceSpawnSpec
{
	TArray<FString> kkw_labels;
	EFPSJanggiPieceRole kkw_role;
	const TCHAR* kkw_mesh_path;
	const TCHAR* kkw_idle_animation_path;
	const TCHAR* kkw_move_animation_path;
	const TCHAR* kkw_attack_animation_path;
	const TCHAR* kkw_run_animation_path;
	FVector kkw_fallback_offset;
};

FString NormalizeActorLabel(const FString& kkw_label)
{
	FString kkw_result = kkw_label.ToLower();
	kkw_result.ReplaceInline(TEXT(" "), TEXT(""));
	kkw_result.ReplaceInline(TEXT("_"), TEXT(""));
	kkw_result.ReplaceInline(TEXT("-"), TEXT(""));
	return kkw_result;
}
}

AFPSJanggiAbilityPlayerController::AFPSJanggiAbilityPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	bAutoManageActiveCameraTarget = false;
}

void AFPSJanggiAbilityPlayerController::BeginPlay()
{
	Super::BeginPlay();
	EnsureScriptDirectoryActor();
	BuildPlayablePieces();
	SelectPieceIndex(0);
}

void AFPSJanggiAbilityPlayerController::Tick(float kkw_delta_seconds)
{
	Super::Tick(kkw_delta_seconds);
	SyncSourcePiecePlacements();
}

void AFPSJanggiAbilityPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction(TEXT("SelectPiece1"), IE_Pressed, this, &AFPSJanggiAbilityPlayerController::SelectPiece1);
	InputComponent->BindAction(TEXT("SelectPiece2"), IE_Pressed, this, &AFPSJanggiAbilityPlayerController::SelectPiece2);
	InputComponent->BindAction(TEXT("SelectPiece3"), IE_Pressed, this, &AFPSJanggiAbilityPlayerController::SelectPiece3);
	InputComponent->BindAction(TEXT("SelectPiece4"), IE_Pressed, this, &AFPSJanggiAbilityPlayerController::SelectPiece4);
	InputComponent->BindAction(TEXT("SelectPiece5"), IE_Pressed, this, &AFPSJanggiAbilityPlayerController::SelectPiece5);
	InputComponent->BindAction(TEXT("SelectPiece6"), IE_Pressed, this, &AFPSJanggiAbilityPlayerController::SelectPiece6);
	InputComponent->BindAction(TEXT("ToggleCameraMode"), IE_Pressed, this, &AFPSJanggiAbilityPlayerController::ToggleCameraMode);
}

void AFPSJanggiAbilityPlayerController::BuildPlayablePieces()
{
	if (!GetWorld() || kkw_controlled_pieces.Num() > 0)
	{
		return;
	}

	const FVector kkw_center = FindFallbackCenter();
	const TArray<FPieceSpawnSpec> kkw_specs = {
		{{TEXT("SkeletalMeshActor"), TEXT("SkeletonMeshActor")}, EFPSJanggiPieceRole::Cannon, TEXT("/Game/User/Charactor/Cannon_R/SK_Cannon_R_CharacterWeapon.SK_Cannon_R_CharacterWeapon"), TEXT("/Game/User/Charactor/Cannon_R/SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_Idle_canon.SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_Idle_canon"), TEXT("/Game/User/Charactor/Cannon_R/SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_Move_canon.SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_Move_canon"), TEXT("/Game/User/Charactor/Cannon_R/SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_attack_canon.SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_attack_canon"), TEXT("/Game/User/Charactor/Cannon_R/SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_run_canon.SK_Cannon_R_CharacterWeaponMP_Piece_06_Armature_run_canon"), FVector(-420.0f, -260.0f, 170.0f)},
		{{TEXT("SkeletalMeshActor2"), TEXT("SkeletonMeshActor2")}, EFPSJanggiPieceRole::Cannon, TEXT("/Game/User/Charactor/Cannon_B/SK_Cannon_B_CharacterWeapon.SK_Cannon_B_CharacterWeapon"), TEXT("/Game/User/Charactor/Cannon_B/SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_Idle_canon.SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_Idle_canon"), TEXT("/Game/User/Charactor/Cannon_B/SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_Move_canon.SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_Move_canon"), TEXT("/Game/User/Charactor/Cannon_B/SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_attack_canon.SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_attack_canon"), TEXT("/Game/User/Charactor/Cannon_B/SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_run_canon.SK_Cannon_B_CharacterWeaponMP_Piece_05_Armature_run_canon"), FVector(-420.0f, 260.0f, 170.0f)},
		{{TEXT("SkeletalMeshActor3"), TEXT("SkeletonMeshActor3")}, EFPSJanggiPieceRole::Guard, TEXT("/Game/User/Charactor/Guard_R/SK_Guard_R_CharacterWeapon.SK_Guard_R_CharacterWeapon"), TEXT("/Game/User/Charactor/Guard_R/SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_Idle.SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_Idle"), TEXT("/Game/User/Charactor/Guard_R/SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_Move.SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_Move"), TEXT("/Game/User/Charactor/Guard_R/SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_attack_sheid.SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_attack_sheid"), TEXT("/Game/User/Charactor/Guard_R/SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_run.SK_Guard_R_CharacterWeaponMP_Piece_14_Armature_run"), FVector(0.0f, -260.0f, 170.0f)},
		{{TEXT("SkeletalMeshActor4"), TEXT("SkeletonMeshActor4")}, EFPSJanggiPieceRole::Guard, TEXT("/Game/User/Charactor/Guard_B/SK_Guard_B_CharacterWeapon.SK_Guard_B_CharacterWeapon"), TEXT("/Game/User/Charactor/Guard_B/SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_Idle.SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_Idle"), TEXT("/Game/User/Charactor/Guard_B/SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_Move.SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_Move"), TEXT("/Game/User/Charactor/Guard_B/SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_attack_sheid.SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_attack_sheid"), TEXT("/Game/User/Charactor/Guard_B/SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_run.SK_Guard_B_CharacterWeaponMP_Piece_13_Armature_run"), FVector(0.0f, 260.0f, 170.0f)},
		{{TEXT("SkeletalMeshActor5"), TEXT("SkeletonMeshActor5")}, EFPSJanggiPieceRole::Chariot, TEXT("/Game/User/Charactor/Chariot_R/SK_Chariot_R_CharacterWeapon.SK_Chariot_R_CharacterWeapon"), TEXT("/Game/User/Charactor/Chariot_R/SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_Idle.SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_Idle"), TEXT("/Game/User/Charactor/Chariot_R/SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_Move.SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_Move"), TEXT("/Game/User/Charactor/Chariot_R/SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_attack_ammor.SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_attack_ammor"), TEXT("/Game/User/Charactor/Chariot_R/SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_run.SK_Chariot_R_CharacterWeaponMP_Piece_04_Armature_run"), FVector(420.0f, -260.0f, 170.0f)},
		{{TEXT("SkeletalMeshActor6"), TEXT("SkeletonMeshActor6")}, EFPSJanggiPieceRole::Chariot, TEXT("/Game/User/Charactor/Chariot_B/SK_Chariot_B_CharacterWeapon.SK_Chariot_B_CharacterWeapon"), TEXT("/Game/User/Charactor/Chariot_B/SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_Idle.SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_Idle"), TEXT("/Game/User/Charactor/Chariot_B/SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_Move.SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_Move"), TEXT("/Game/User/Charactor/Chariot_B/SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_attack_ammor.SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_attack_ammor"), TEXT("/Game/User/Charactor/Chariot_B/SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_run.SK_Chariot_B_CharacterWeaponMP_Piece_03_Armature_run"), FVector(420.0f, 260.0f, 170.0f)}
	};

	for (int32 kkw_spawn_index = 0; kkw_spawn_index < kkw_specs.Num(); ++kkw_spawn_index)
	{
		const FPieceSpawnSpec& kkw_spec = kkw_specs[kkw_spawn_index];
		AActor* kkw_source_actor = FindActorByLabels(kkw_spec.kkw_labels);
		USkeletalMeshComponent* kkw_source_mesh_component = kkw_source_actor ? Cast<USkeletalMeshComponent>(kkw_source_actor->GetComponentByClass(USkeletalMeshComponent::StaticClass())) : nullptr;
		USkeletalMesh* kkw_configured_mesh = LoadMesh(kkw_spec.kkw_mesh_path);
		USkeletalMesh* kkw_source_mesh = kkw_source_mesh_component ? kkw_source_mesh_component->GetSkeletalMeshAsset() : nullptr;
		USkeletalMesh* kkw_mesh = kkw_configured_mesh ? kkw_configured_mesh : kkw_source_mesh;
		const bool kkw_b_use_source_placement = kkw_source_mesh_component && kkw_mesh;

		const FTransform kkw_spawn_transform = kkw_b_use_source_placement
			? kkw_source_mesh_component->GetComponentTransform()
			: FTransform(FRotator::ZeroRotator, kkw_center + kkw_spec.kkw_fallback_offset, FVector::OneVector);

		FActorSpawnParameters kkw_spawn_params;
		kkw_spawn_params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AFPSJanggiAbilityCharacter* kkw_piece = GetWorld()->SpawnActor<AFPSJanggiAbilityCharacter>(AFPSJanggiAbilityCharacter::StaticClass(), kkw_spawn_transform, kkw_spawn_params);
		if (!kkw_piece)
		{
			continue;
		}

		const FString kkw_piece_actor_label = FString::Printf(TEXT("FPSJanggiAbilityCharacter%d"), kkw_spawn_index);
#if WITH_EDITOR
		kkw_piece->SetActorLabel(kkw_piece_actor_label);
#endif
		kkw_piece->Tags.AddUnique(FName(*kkw_piece_actor_label));
		kkw_piece->ConfigurePiece(kkw_spec.kkw_role, kkw_mesh, kkw_spawn_transform, kkw_b_use_source_placement);
		kkw_piece->ConfigureAnimations(
			LoadAnimation(kkw_spec.kkw_idle_animation_path),
			LoadAnimation(kkw_spec.kkw_move_animation_path),
			LoadAnimation(kkw_spec.kkw_attack_animation_path),
			LoadAnimation(kkw_spec.kkw_run_animation_path));
		kkw_controlled_pieces.Add(kkw_piece);
		kkw_source_piece_actors.Add(kkw_source_actor);
		kkw_last_source_piece_transforms.Add(kkw_source_actor ? GetSourcePlacementTransform(kkw_source_actor) : kkw_spawn_transform);

		if (kkw_source_actor)
		{
			kkw_source_actor->SetActorHiddenInGame(true);
			kkw_source_actor->SetActorEnableCollision(false);
		}
	}
}

void AFPSJanggiAbilityPlayerController::SyncSourcePiecePlacements()
{
	if (kkw_controlled_pieces.Num() != kkw_source_piece_actors.Num() || kkw_controlled_pieces.Num() != kkw_last_source_piece_transforms.Num())
	{
		return;
	}

	for (int32 kkw_index = 0; kkw_index < kkw_controlled_pieces.Num(); ++kkw_index)
	{
		AFPSJanggiAbilityCharacter* kkw_piece = kkw_controlled_pieces[kkw_index];
		AActor* kkw_source_actor = kkw_source_piece_actors[kkw_index];
		if (!IsValid(kkw_piece) || !IsValid(kkw_source_actor))
		{
			continue;
		}

		const FTransform kkw_source_transform = GetSourcePlacementTransform(kkw_source_actor);
		if (kkw_source_transform.Equals(kkw_last_source_piece_transforms[kkw_index], 0.05f))
		{
			continue;
		}

		kkw_piece->ApplySourcePlacementTransform(kkw_source_transform, true, false);
		kkw_last_source_piece_transforms[kkw_index] = kkw_source_transform;

		if (GetPawn() == kkw_piece)
		{
			SetControlRotation(FRotator(0.0f, kkw_piece->GetActorRotation().Yaw, 0.0f));
			SetViewTarget(kkw_piece);
			kkw_piece->ForceFirstPersonView();
		}
	}
}

void AFPSJanggiAbilityPlayerController::EnsureScriptDirectoryActor()
{
	if (!GetWorld() || FindActorByLabels({TEXT("KKW_Scripts_Folder")}))
	{
		return;
	}

	FActorSpawnParameters kkw_spawn_params;
	kkw_spawn_params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AKKWProjectScriptsActor* kkw_scripts_actor = GetWorld()->SpawnActor<AKKWProjectScriptsActor>(
		AKKWProjectScriptsActor::StaticClass(),
		FindFallbackCenter(),
		FRotator::ZeroRotator,
		kkw_spawn_params);

#if WITH_EDITOR
	if (kkw_scripts_actor)
	{
		kkw_scripts_actor->SetActorLabel(TEXT("KKW_Scripts_Folder"));
	}
#endif
}

void AFPSJanggiAbilityPlayerController::SelectPieceIndex(int32 kkw_index)
{
	if (!kkw_controlled_pieces.IsValidIndex(kkw_index) || !IsValid(kkw_controlled_pieces[kkw_index]))
	{
		return;
	}

	AFPSJanggiAbilityCharacter* kkw_piece = kkw_controlled_pieces[kkw_index];
	Possess(kkw_piece);
	for (AFPSJanggiAbilityCharacter* kkw_controlled_piece : kkw_controlled_pieces)
	{
		if (IsValid(kkw_controlled_piece))
		{
			kkw_controlled_piece->SetFirstPersonActive(kkw_controlled_piece == kkw_piece);
		}
	}

	SetControlRotation(FRotator(0.0f, kkw_piece->GetActorRotation().Yaw, 0.0f));
	SetViewTarget(kkw_piece);
	kkw_piece->ForceFirstPersonView();
}

void AFPSJanggiAbilityPlayerController::SelectPiece1()
{
	SelectPieceIndex(0);
}

void AFPSJanggiAbilityPlayerController::SelectPiece2()
{
	SelectPieceIndex(1);
}

void AFPSJanggiAbilityPlayerController::SelectPiece3()
{
	SelectPieceIndex(2);
}

void AFPSJanggiAbilityPlayerController::SelectPiece4()
{
	SelectPieceIndex(3);
}

void AFPSJanggiAbilityPlayerController::SelectPiece5()
{
	SelectPieceIndex(4);
}

void AFPSJanggiAbilityPlayerController::SelectPiece6()
{
	SelectPieceIndex(5);
}

void AFPSJanggiAbilityPlayerController::ToggleCameraMode()
{
	if (AFPSJanggiAbilityCharacter* kkw_piece = Cast<AFPSJanggiAbilityCharacter>(GetPawn()))
	{
		kkw_piece->ToggleCameraMode();
	}
}

AActor* AFPSJanggiAbilityPlayerController::FindActorByLabels(const TArray<FString>& kkw_labels) const
{
	TArray<AActor*> kkw_actors;
	UGameplayStatics::GetAllActorsOfClass(this, AActor::StaticClass(), kkw_actors);

	TSet<FString> kkw_normalized_labels;
	for (const FString& kkw_label : kkw_labels)
	{
		kkw_normalized_labels.Add(NormalizeActorLabel(kkw_label));
	}

	for (AActor* kkw_actor : kkw_actors)
	{
		if (!kkw_actor)
		{
			continue;
		}

		for (const FName& kkw_tag : kkw_actor->Tags)
		{
			if (kkw_normalized_labels.Contains(NormalizeActorLabel(kkw_tag.ToString())))
			{
				return kkw_actor;
			}
		}

		const FString kkw_normalized_actor_label = NormalizeActorLabel(kkw_actor->GetActorNameOrLabel());
		const FString kkw_actor_name = NormalizeActorLabel(kkw_actor->GetName());
		if (kkw_normalized_labels.Contains(kkw_normalized_actor_label) || kkw_normalized_labels.Contains(kkw_actor_name))
		{
			return kkw_actor;
		}
	}

	return nullptr;
}

FTransform AFPSJanggiAbilityPlayerController::GetSourcePlacementTransform(AActor* kkw_source_actor) const
{
	if (!kkw_source_actor)
	{
		return FTransform::Identity;
	}

	if (USkeletalMeshComponent* kkw_source_mesh_component = Cast<USkeletalMeshComponent>(kkw_source_actor->GetComponentByClass(USkeletalMeshComponent::StaticClass())))
	{
		return kkw_source_mesh_component->GetComponentTransform();
	}

	return kkw_source_actor->GetActorTransform();
}

FVector AFPSJanggiAbilityPlayerController::FindFallbackCenter() const
{
	TArray<AActor*> kkw_actors;
	UGameplayStatics::GetAllActorsOfClass(this, AActor::StaticClass(), kkw_actors);
	for (AActor* kkw_actor : kkw_actors)
	{
		if (kkw_actor && NormalizeActorLabel(kkw_actor->GetActorNameOrLabel()).Contains(TEXT("janggiboard")))
		{
			return kkw_actor->GetActorLocation();
		}
	}

	return FVector::ZeroVector;
}

USkeletalMesh* AFPSJanggiAbilityPlayerController::LoadMesh(const TCHAR* kkw_mesh_path) const
{
	return LoadObject<USkeletalMesh>(nullptr, kkw_mesh_path);
}

UAnimSequence* AFPSJanggiAbilityPlayerController::LoadAnimation(const TCHAR* kkw_animation_path) const
{
	return LoadObject<UAnimSequence>(nullptr, kkw_animation_path);
}
