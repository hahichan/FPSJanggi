// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSJanggiAbilityCharacter.h"

#include "Animation/AnimSequence.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "FPSJanggiAbilityProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
FString KKWNormalizeActorText(const FString& kkw_text)
{
	FString kkw_result = kkw_text.ToLower();
	kkw_result.ReplaceInline(TEXT(" "), TEXT(""));
	kkw_result.ReplaceInline(TEXT("_"), TEXT(""));
	kkw_result.ReplaceInline(TEXT("-"), TEXT(""));
	return kkw_result;
}

FString KKWGroundActorText(const AActor* kkw_actor)
{
	return kkw_actor ? KKWNormalizeActorText(kkw_actor->GetActorNameOrLabel() + TEXT(" ") + kkw_actor->GetName()) : FString();
}
}

AFPSJanggiAbilityCharacter::AFPSJanggiAbilityCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(55.0f, 110.0f);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	kkw_camera_boom = CreateDefaultSubobject<USpringArmComponent>(TEXT("kkw_camera_boom"));
	kkw_camera_boom->SetupAttachment(RootComponent);
	kkw_camera_boom->TargetArmLength = 0.0f;
	kkw_camera_boom->bUsePawnControlRotation = true;
	kkw_camera_boom->bDoCollisionTest = false;
	kkw_camera_boom->bEnableCameraLag = false;

	kkw_follow_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("kkw_follow_camera"));
	kkw_follow_camera->SetupAttachment(RootComponent);
	kkw_follow_camera->bUsePawnControlRotation = true;
	kkw_follow_camera->SetFieldOfView(90.0f);

	kkw_first_person_mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("kkw_first_person_mesh"));
	kkw_first_person_mesh->SetupAttachment(kkw_follow_camera);
	kkw_first_person_mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	kkw_first_person_mesh->SetCanEverAffectNavigation(false);
	kkw_first_person_mesh->SetCastShadow(false);
	kkw_first_person_mesh->bReceivesDecals = false;
	kkw_first_person_mesh->SetVisibility(false, true);
	kkw_first_person_mesh->SetRelativeRotation(kkw_first_person_mesh_rotation);
	kkw_first_person_mesh->SetRelativeScale3D(FVector(kkw_first_person_visual_scale));

	kkw_left_fist_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("kkw_left_fist_mesh"));
	kkw_left_fist_mesh->SetupAttachment(kkw_follow_camera);
	kkw_left_fist_mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	kkw_left_fist_mesh->SetCanEverAffectNavigation(false);
	kkw_left_fist_mesh->SetCastShadow(false);
	kkw_left_fist_mesh->bReceivesDecals = false;
	kkw_left_fist_mesh->SetVisibility(false, true);

	kkw_right_fist_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("kkw_right_fist_mesh"));
	kkw_right_fist_mesh->SetupAttachment(kkw_follow_camera);
	kkw_right_fist_mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	kkw_right_fist_mesh->SetCanEverAffectNavigation(false);
	kkw_right_fist_mesh->SetCastShadow(false);
	kkw_right_fist_mesh->bReceivesDecals = false;
	kkw_right_fist_mesh->SetVisibility(false, true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> kkw_fist_mesh_asset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (kkw_fist_mesh_asset.Succeeded())
	{
		kkw_left_fist_mesh->SetStaticMesh(kkw_fist_mesh_asset.Object);
		kkw_right_fist_mesh->SetStaticMesh(kkw_fist_mesh_asset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> kkw_fist_material(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (kkw_fist_material.Succeeded())
	{
		kkw_left_fist_mesh->SetMaterial(0, kkw_fist_material.Object);
		kkw_right_fist_mesh->SetMaterial(0, kkw_fist_material.Object);
	}

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = 460.0f;
	GetCharacterMovement()->JumpZVelocity = 650.0f;

	kkw_projectile_class = AFPSJanggiAbilityProjectile::StaticClass();

	static ConstructorHelpers::FObjectFinder<USoundBase> kkw_bang_sound(TEXT("/Engine/EngineSounds/1kSineTonePing.1kSineTonePing"));
	if (kkw_bang_sound.Succeeded())
	{
		kkw_cannon_jump_bang_sound = kkw_bang_sound.Object;
	}

	ApplyRoleStats();
}

void AFPSJanggiAbilityCharacter::Tick(float kkw_delta_seconds)
{
	Super::Tick(kkw_delta_seconds);

	if (!kkw_b_action_animation_active)
	{
		RefreshMovementAnimation();
	}
}

void AFPSJanggiAbilityCharacter::SetupPlayerInputComponent(UInputComponent* kkw_player_input_component)
{
	Super::SetupPlayerInputComponent(kkw_player_input_component);

	kkw_player_input_component->BindAxis(TEXT("MoveForward"), this, &AFPSJanggiAbilityCharacter::MoveForward);
	kkw_player_input_component->BindAxis(TEXT("MoveRight"), this, &AFPSJanggiAbilityCharacter::MoveRight);
	kkw_player_input_component->BindAxis(TEXT("Turn"), this, &AFPSJanggiAbilityCharacter::Turn);
	kkw_player_input_component->BindAxis(TEXT("LookUp"), this, &AFPSJanggiAbilityCharacter::LookUp);

	kkw_player_input_component->BindAction(TEXT("PrimaryAbility"), IE_Pressed, this, &AFPSJanggiAbilityCharacter::PrimaryPressed);
	kkw_player_input_component->BindAction(TEXT("SecondaryAbility"), IE_Pressed, this, &AFPSJanggiAbilityCharacter::SecondaryPressed);
	kkw_player_input_component->BindAction(TEXT("SecondaryAbility"), IE_Released, this, &AFPSJanggiAbilityCharacter::SecondaryReleased);
	kkw_player_input_component->BindAction(TEXT("AbilityE"), IE_Pressed, this, &AFPSJanggiAbilityCharacter::AbilityEPressed);
	kkw_player_input_component->BindAction(TEXT("AbilityQ"), IE_Pressed, this, &AFPSJanggiAbilityCharacter::AbilityQPressed);
}

float AFPSJanggiAbilityCharacter::TakeDamage(float kkw_damage_amount, FDamageEvent const& kkw_damage_event, AController* kkw_event_instigator, AActor* kkw_damage_causer)
{
	if (kkw_b_parrying)
	{
		ShowAbilityText(TEXT("Parry: attack ignored"), FColor::Cyan);
		return 0.0f;
	}

	const float kkw_final_damage = kkw_b_guarding ? kkw_damage_amount * (1.0f - kkw_guard_damage_reduction) : kkw_damage_amount;
	kkw_health = FMath::Clamp(kkw_health - kkw_final_damage, 0.0f, kkw_max_health);
	kkw_damage_taken_total += kkw_final_damage;

	ShowAbilityText(FString::Printf(TEXT("%s damage %.0f / HP %.0f"), *kkw_piece_label.ToString(), kkw_final_damage, kkw_health), FColor::Red);
	return kkw_final_damage;
}

void AFPSJanggiAbilityCharacter::CalcCamera(float kkw_delta_time, FMinimalViewInfo& kkw_out_result)
{
	if (kkw_follow_camera)
	{
		kkw_follow_camera->GetCameraView(kkw_delta_time, kkw_out_result);
		return;
	}

	Super::CalcCamera(kkw_delta_time, kkw_out_result);
}

void AFPSJanggiAbilityCharacter::ConfigurePiece(EFPSJanggiPieceRole kkw_in_role, USkeletalMesh* kkw_in_mesh, const FTransform& kkw_source_mesh_transform, bool kkw_b_use_source_placement)
{
	kkw_piece_role = kkw_in_role;
	ApplyRoleStats();

	if (kkw_in_mesh)
	{
		GetMesh()->SetSkeletalMesh(kkw_in_mesh);
		kkw_first_person_mesh->SetSkeletalMesh(kkw_in_mesh);
		RefreshFirstPersonMeshSections();
	}

	ApplySourcePlacementTransform(kkw_source_mesh_transform, kkw_b_use_source_placement);
	SetFirstPersonActive(false);
}

void AFPSJanggiAbilityCharacter::ApplySourcePlacementTransform(const FTransform& kkw_source_mesh_transform, bool kkw_b_use_source_placement, bool kkw_b_snap_to_ground)
{
	USkeletalMesh* kkw_mesh_asset = GetMesh()->GetSkeletalMeshAsset();
	float kkw_height_fit_scale = kkw_character_visual_scale;
	if (kkw_mesh_asset)
	{
		const FBoxSphereBounds kkw_bounds = kkw_mesh_asset->GetBounds();
		const float kkw_source_visual_height = FMath::Max(1.0f, kkw_bounds.BoxExtent.Z * 2.0f);
		kkw_height_fit_scale *= kkw_target_visual_height / kkw_source_visual_height;
	}

	const FVector kkw_piece_scale = kkw_source_mesh_transform.GetScale3D() * kkw_height_fit_scale;
	GetMesh()->SetWorldScale3D(kkw_piece_scale);
	RefreshFirstPersonMeshTransform();
	const FRotator kkw_source_rotation = kkw_source_mesh_transform.Rotator();
	const FRotator kkw_source_yaw_rotation(0.0f, kkw_source_rotation.Yaw, 0.0f);
	const FRotator kkw_actor_rotation(0.0f, kkw_source_yaw_rotation.Yaw + kkw_piece_forward_yaw_offset, 0.0f);
	SetActorRotation(kkw_actor_rotation);
	kkw_mesh_relative_rotation = (kkw_actor_rotation.Quaternion().Inverse() * kkw_source_yaw_rotation.Quaternion()).Rotator();
	CenterMeshOnCapsule();

	if (kkw_b_use_source_placement && kkw_mesh_asset)
	{
		const FBoxSphereBounds kkw_bounds = kkw_mesh_asset->GetBounds();
		const float kkw_half_height = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const FVector kkw_center_xy = kkw_source_mesh_transform.TransformPosition(FVector(kkw_bounds.Origin.X, kkw_bounds.Origin.Y, 0.0f));
		const FVector kkw_source_bottom = kkw_source_mesh_transform.TransformPosition(FVector(kkw_bounds.Origin.X, kkw_bounds.Origin.Y, kkw_bounds.Origin.Z - kkw_bounds.BoxExtent.Z));
		const FVector kkw_source_location = kkw_source_mesh_transform.GetLocation();
		FVector kkw_desired_location(kkw_center_xy.X, kkw_center_xy.Y, kkw_source_bottom.Z + kkw_half_height + kkw_ground_snap_offset);
		if (kkw_b_snap_to_ground)
		{
			const FVector kkw_trace_start(kkw_desired_location.X, kkw_desired_location.Y, kkw_source_location.Z + 2000.0f);
			const FVector kkw_trace_end(kkw_desired_location.X, kkw_desired_location.Y, kkw_source_location.Z - 8000.0f);
			FindGroundSnapLocation(kkw_trace_start, kkw_trace_end, kkw_half_height, kkw_desired_location);
		}

		SetActorLocation(kkw_desired_location, false);
	}
}

void AFPSJanggiAbilityCharacter::ConfigureAnimations(UAnimSequence* kkw_in_idle_animation, UAnimSequence* kkw_in_move_animation, UAnimSequence* kkw_in_attack_animation, UAnimSequence* kkw_in_run_animation)
{
	kkw_idle_animation = kkw_in_idle_animation;
	kkw_move_animation = kkw_in_move_animation;
	kkw_attack_animation = kkw_in_attack_animation;
	kkw_run_animation = kkw_in_run_animation;

	if (kkw_idle_animation || kkw_move_animation || kkw_attack_animation || kkw_run_animation)
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		kkw_first_person_mesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		RefreshMovementAnimation();
	}
}

void AFPSJanggiAbilityCharacter::MoveForward(float kkw_value)
{
	if (!FMath::IsNearlyZero(kkw_value) && Controller)
	{
		const FRotator kkw_yaw_rotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		AddMovementInput(FRotationMatrix(kkw_yaw_rotation).GetUnitAxis(EAxis::X), kkw_value);
	}
}

void AFPSJanggiAbilityCharacter::MoveRight(float kkw_value)
{
	if (!FMath::IsNearlyZero(kkw_value) && Controller)
	{
		const FRotator kkw_yaw_rotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		AddMovementInput(FRotationMatrix(kkw_yaw_rotation).GetUnitAxis(EAxis::Y), kkw_value);
	}
}

void AFPSJanggiAbilityCharacter::Turn(float kkw_value)
{
	AddControllerYawInput(kkw_value);
}

void AFPSJanggiAbilityCharacter::LookUp(float kkw_value)
{
	AddControllerPitchInput(kkw_value);
}

void AFPSJanggiAbilityCharacter::PrimaryPressed()
{
	PlayActionAnimation(kkw_attack_animation);

	switch (kkw_piece_role)
	{
	case EFPSJanggiPieceRole::Cannon:
		SpawnProjectileShot(GetAimDirection(), 55.0f, 2450.0f, 220.0f, FColor::Orange);
		ShowAbilityText(TEXT("포 artillery shot"), FColor::Orange);
		break;
	case EFPSJanggiPieceRole::Guard:
		DealMeleeDamage(35.0f, 230.0f, 130.0f, 1400.0f, FColor::Cyan, TEXT("사 shield push"));
		break;
	case EFPSJanggiPieceRole::Chariot:
		DealMeleeDamage(45.0f, 190.0f, 95.0f, 900.0f, FColor::Silver, TEXT("차 punch"));
		break;
	}
	ForceFirstPersonView();
}

void AFPSJanggiAbilityCharacter::SecondaryPressed()
{
	PlayActionAnimation(kkw_attack_animation);

	switch (kkw_piece_role)
	{
	case EFPSJanggiPieceRole::Cannon:
	{
		const FVector kkw_aim_direction = GetAimDirection();
		const FRotator kkw_aim_rotation = kkw_aim_direction.Rotation();
		constexpr int32 kkw_shot_count = 6;
		constexpr float kkw_spread_degrees = 24.0f;
		for (int32 kkw_index = 0; kkw_index < kkw_shot_count; ++kkw_index)
		{
			const float kkw_alpha = kkw_shot_count > 1 ? static_cast<float>(kkw_index) / static_cast<float>(kkw_shot_count - 1) : 0.5f;
			const float kkw_yaw_offset = FMath::Lerp(-kkw_spread_degrees * 0.5f, kkw_spread_degrees * 0.5f, kkw_alpha);
			SpawnProjectileShot((kkw_aim_rotation + FRotator(0.0f, kkw_yaw_offset, 0.0f)).Vector(), 22.0f, 2200.0f, 165.0f, FColor::Yellow);
		}
		ShowAbilityText(TEXT("포 fan fire"), FColor::Yellow);
		break;
	}
	case EFPSJanggiPieceRole::Guard:
		kkw_b_parrying = true;
		ShowAbilityText(TEXT("사 parry"), FColor::Cyan);
		GetWorldTimerManager().SetTimer(kkw_parry_timer_handle, this, &AFPSJanggiAbilityCharacter::EndParry, 1.0f, false);
		break;
	case EFPSJanggiPieceRole::Chariot:
		kkw_b_guarding = true;
		ShowAbilityText(TEXT("차 guard"), FColor::Green);
		break;
	}
	ForceFirstPersonView();
}

void AFPSJanggiAbilityCharacter::SecondaryReleased()
{
	if (kkw_piece_role == EFPSJanggiPieceRole::Guard)
	{
		EndParry();
	}
	else if (kkw_piece_role == EFPSJanggiPieceRole::Chariot)
	{
		kkw_b_guarding = false;
	}
	ForceFirstPersonView();
}

void AFPSJanggiAbilityCharacter::AbilityEPressed()
{
	PlayActionAnimation(kkw_attack_animation);

	switch (kkw_piece_role)
	{
	case EFPSJanggiPieceRole::Cannon:
	{
		const FVector kkw_origin = GetActorLocation() - FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.8f);
		DealRadialAbilityDamage(38.0f, 300.0f, kkw_origin, FColor::Orange, TEXT("포 blast jump"));
		PlayCannonJumpBang(kkw_origin);
		LaunchCharacter(GetAimDirection() * 950.0f + FVector(0.0f, 0.0f, 880.0f), true, true);
		break;
	}
	case EFPSJanggiPieceRole::Guard:
		DealRadialAbilityDamage(80.0f + kkw_damage_taken_total * 1.35f, 520.0f, GetActorLocation(), FColor::Red, TEXT("사 self-destruct"));
		kkw_health = FMath::Max(1.0f, kkw_max_health * 0.15f);
		kkw_damage_taken_total = 0.0f;
		break;
	case EFPSJanggiPieceRole::Chariot:
		if (!kkw_b_charging)
		{
			kkw_b_charging = true;
			LaunchCharacter(GetAimDirection() * 1800.0f + FVector(0.0f, 0.0f, 80.0f), true, true);
			GetWorldTimerManager().SetTimer(kkw_charge_tick_timer_handle, this, &AFPSJanggiAbilityCharacter::ChargeTick, 0.08f, true);
			GetWorldTimerManager().SetTimer(kkw_charge_end_timer_handle, this, &AFPSJanggiAbilityCharacter::EndCharge, 0.45f, false);
			ShowAbilityText(TEXT("차 charge"), FColor::Silver);
		}
		break;
	}
	ForceFirstPersonView();
}

void AFPSJanggiAbilityCharacter::AbilityQPressed()
{
	PlayActionAnimation(kkw_attack_animation);

	switch (kkw_piece_role)
	{
	case EFPSJanggiPieceRole::Cannon:
		DealRadialAbilityDamage(85.0f, 6000.0f, GetActorLocation(), FColor::Red, TEXT("포 full-range flame attack"));
		break;
	case EFPSJanggiPieceRole::Chariot:
		BlinkForward();
		break;
	case EFPSJanggiPieceRole::Guard:
		ShowAbilityText(TEXT("사 Q empty"), FColor::White);
		break;
	}
	ForceFirstPersonView();
}

void AFPSJanggiAbilityCharacter::CenterMeshOnCapsule()
{
	USkeletalMesh* kkw_mesh_asset = GetMesh()->GetSkeletalMeshAsset();
	if (!kkw_mesh_asset)
	{
		return;
	}

	const FBoxSphereBounds kkw_bounds = kkw_mesh_asset->GetBounds();
	const FVector kkw_scale = GetMesh()->GetRelativeScale3D();
	const float kkw_visual_height = FMath::Max(8.0f, kkw_bounds.BoxExtent.Z * 2.0f * kkw_scale.Z);
	const float kkw_visual_radius = FMath::Max(kkw_bounds.BoxExtent.X * kkw_scale.X, kkw_bounds.BoxExtent.Y * kkw_scale.Y);
	const float kkw_radius = FMath::Clamp(kkw_visual_radius * 0.45f + 18.0f, 30.0f, 160.0f);
	const float kkw_half_height = FMath::Clamp(kkw_visual_height * 0.5f + 18.0f, 60.0f, 190.0f);
	const float kkw_mesh_min_z = (kkw_bounds.Origin.Z - kkw_bounds.BoxExtent.Z) * kkw_scale.Z;
	const float kkw_camera_target_z = FMath::Clamp(-kkw_half_height + kkw_visual_height * 1.15f, -kkw_half_height + 8.0f, kkw_half_height * 0.75f);
	const float kkw_camera_forward_offset = FMath::Clamp(kkw_radius * 0.35f, 4.0f, 18.0f);
	kkw_first_person_camera_location = FVector(kkw_camera_forward_offset, 0.0f, kkw_camera_target_z);
	kkw_third_person_camera_location = FVector(-(kkw_radius + 520.0f), 0.0f, FMath::Clamp(kkw_half_height * 1.15f, 150.0f, 300.0f));

	GetCapsuleComponent()->SetCapsuleSize(kkw_radius, kkw_half_height, true);
	GetMesh()->SetRelativeLocation(FVector(-kkw_bounds.Origin.X * kkw_scale.X, -kkw_bounds.Origin.Y * kkw_scale.Y, -kkw_half_height - kkw_mesh_min_z));
	GetMesh()->SetRelativeRotation(kkw_mesh_relative_rotation);

	kkw_camera_boom->TargetOffset = FVector(0.0f, 0.0f, kkw_camera_target_z);
	kkw_camera_boom->SocketOffset = FVector::ZeroVector;
	kkw_camera_boom->TargetArmLength = 0.0f;
	kkw_follow_camera->SetRelativeLocation(kkw_first_person_camera_location);
	kkw_follow_camera->SetRelativeRotation(FRotator::ZeroRotator);
	ApplyCameraMode();
}

bool AFPSJanggiAbilityCharacter::FindGroundSnapLocation(const FVector& kkw_trace_start, const FVector& kkw_trace_end, float kkw_half_height, FVector& kkw_out_location) const
{
	if (!GetWorld())
	{
		return false;
	}

	FCollisionQueryParams kkw_ground_params(SCENE_QUERY_STAT(JanggiGroundSnap), false, this);
	kkw_ground_params.AddIgnoredActor(this);

	TArray<FHitResult> kkw_hits;
	if (!GetWorld()->LineTraceMultiByChannel(kkw_hits, kkw_trace_start, kkw_trace_end, ECC_WorldStatic, kkw_ground_params))
	{
		return false;
	}

	const FHitResult* kkw_fallback_hit = nullptr;
	for (const FHitResult& kkw_hit : kkw_hits)
	{
		if (!kkw_hit.bBlockingHit || kkw_hit.ImpactNormal.Z < 0.45f || IsRejectedGroundHit(kkw_hit))
		{
			continue;
		}

		if (IsPreferredGroundHit(kkw_hit))
		{
			kkw_out_location.Z = kkw_hit.ImpactPoint.Z + kkw_half_height + kkw_ground_snap_offset;
			return true;
		}

		if (!kkw_fallback_hit)
		{
			kkw_fallback_hit = &kkw_hit;
		}
	}

	if (kkw_fallback_hit)
	{
		kkw_out_location.Z = kkw_fallback_hit->ImpactPoint.Z + kkw_half_height + kkw_ground_snap_offset;
		return true;
	}

	return false;
}

bool AFPSJanggiAbilityCharacter::IsPreferredGroundHit(const FHitResult& kkw_hit) const
{
	const FString kkw_ground_text = KKWGroundActorText(kkw_hit.GetActor());
	return kkw_ground_text.Contains(TEXT("landscape")) || kkw_ground_text.Contains(TEXT("bppatch")) || kkw_ground_text.Contains(TEXT("janggiboard"));
}

bool AFPSJanggiAbilityCharacter::IsRejectedGroundHit(const FHitResult& kkw_hit) const
{
	const FString kkw_ground_text = KKWGroundActorText(kkw_hit.GetActor());
	return kkw_ground_text.Contains(TEXT("stone")) ||
		kkw_ground_text.Contains(TEXT("rock")) ||
		kkw_ground_text.Contains(TEXT("wall")) ||
		kkw_ground_text.Contains(TEXT("wood")) ||
		kkw_ground_text.Contains(TEXT("cart")) ||
		kkw_ground_text.Contains(TEXT("door")) ||
		kkw_ground_text.Contains(TEXT("torus")) ||
		kkw_ground_text.Contains(TEXT("sword"));
}

void AFPSJanggiAbilityCharacter::ApplyRoleStats()
{
	kkw_b_parrying = false;
	kkw_b_guarding = false;

	switch (kkw_piece_role)
	{
	case EFPSJanggiPieceRole::Cannon:
		kkw_piece_label = FText::FromString(TEXT("포"));
		kkw_max_health = 390.0f;
		GetCharacterMovement()->MaxWalkSpeed = 430.0f;
		break;
	case EFPSJanggiPieceRole::Guard:
		kkw_piece_label = FText::FromString(TEXT("사"));
		kkw_max_health = 360.0f;
		kkw_guard_damage_reduction = 1.0f;
		GetCharacterMovement()->MaxWalkSpeed = 460.0f;
		break;
	case EFPSJanggiPieceRole::Chariot:
		kkw_piece_label = FText::FromString(TEXT("차"));
		kkw_max_health = 520.0f;
		kkw_guard_damage_reduction = 0.85f;
		GetCharacterMovement()->MaxWalkSpeed = 500.0f;
		break;
	}

	kkw_health = kkw_max_health;
}

FVector AFPSJanggiAbilityCharacter::GetAimDirection() const
{
	if (Controller)
	{
		return Controller->GetControlRotation().Vector().GetSafeNormal();
	}

	return GetActorForwardVector();
}

FVector AFPSJanggiAbilityCharacter::GetAbilityOrigin(float kkw_forward_offset) const
{
	if (kkw_follow_camera)
	{
		return kkw_follow_camera->GetComponentLocation() + GetAimDirection() * kkw_forward_offset;
	}

	return GetActorLocation() + FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.25f) + GetAimDirection() * kkw_forward_offset;
}

void AFPSJanggiAbilityCharacter::ForceFirstPersonView()
{
	if (kkw_follow_camera)
	{
		kkw_follow_camera->SetActive(true);
	}

	if (APlayerController* kkw_player_controller = Cast<APlayerController>(GetController()))
	{
		kkw_b_selected_view_target = true;
		ApplyCameraMode();
		kkw_player_controller->SetViewTarget(this);
	}
}

void AFPSJanggiAbilityCharacter::SetFirstPersonActive(bool kkw_b_active)
{
	kkw_b_selected_view_target = kkw_b_active;
	ApplyCameraMode();
}

void AFPSJanggiAbilityCharacter::ToggleCameraMode()
{
	if (!kkw_b_selected_view_target)
	{
		return;
	}

	kkw_b_third_person_active = !kkw_b_third_person_active;
	ApplyCameraMode();
	ForceFirstPersonView();
}

void AFPSJanggiAbilityCharacter::ShowAbilityText(const FString& kkw_message, const FColor& kkw_color) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, kkw_color, kkw_message);
	}
}

void AFPSJanggiAbilityCharacter::DealMeleeDamage(float kkw_damage, float kkw_range, float kkw_radius, float kkw_knockback, const FColor& kkw_debug_color, const FString& kkw_debug_label)
{
	const FVector kkw_start = GetAbilityOrigin(65.0f);
	const FVector kkw_end = kkw_start + GetAimDirection() * kkw_range;

	FCollisionQueryParams kkw_params(SCENE_QUERY_STAT(JanggiMelee), false, this);
	TArray<FHitResult> kkw_hits;
	GetWorld()->SweepMultiByChannel(kkw_hits, kkw_start, kkw_end, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(kkw_radius), kkw_params);

	TSet<AActor*> kkw_damaged_actors;
	for (const FHitResult& kkw_hit : kkw_hits)
	{
		AActor* kkw_hit_actor = kkw_hit.GetActor();
		if (!kkw_hit_actor || kkw_hit_actor == this || kkw_damaged_actors.Contains(kkw_hit_actor))
		{
			continue;
		}

		kkw_damaged_actors.Add(kkw_hit_actor);
		UGameplayStatics::ApplyDamage(kkw_hit_actor, kkw_damage, GetController(), this, UDamageType::StaticClass());

		if (UPrimitiveComponent* kkw_primitive = Cast<UPrimitiveComponent>(kkw_hit.GetComponent()))
		{
			if (kkw_primitive->IsSimulatingPhysics())
			{
				kkw_primitive->AddImpulse(GetAimDirection() * kkw_knockback, NAME_None, true);
			}
		}
	}

	DrawDebugSphere(GetWorld(), kkw_end, kkw_radius, 20, kkw_debug_color, false, 0.8f, 0, 4.0f);
	ShowAbilityText(kkw_debug_label, kkw_debug_color);
}

void AFPSJanggiAbilityCharacter::DealRadialAbilityDamage(float kkw_damage, float kkw_radius, const FVector& kkw_origin, const FColor& kkw_debug_color, const FString& kkw_debug_label)
{
	TArray<AActor*> kkw_ignored_actors;
	kkw_ignored_actors.Add(this);
	UGameplayStatics::ApplyRadialDamage(this, kkw_damage, kkw_origin, kkw_radius, UDamageType::StaticClass(), kkw_ignored_actors, this, GetController(), true);
	DrawDebugSphere(GetWorld(), kkw_origin, kkw_radius, 32, kkw_debug_color, false, 1.2f, 0, 5.0f);
	ShowAbilityText(kkw_debug_label, kkw_debug_color);
}

void AFPSJanggiAbilityCharacter::EndParry()
{
	kkw_b_parrying = false;
	GetWorldTimerManager().ClearTimer(kkw_parry_timer_handle);
}

void AFPSJanggiAbilityCharacter::ChargeTick()
{
	DealMeleeDamage(50.0f, 160.0f, 115.0f, 1600.0f, FColor::Silver, TEXT("차 charge hit"));
}

void AFPSJanggiAbilityCharacter::EndCharge()
{
	kkw_b_charging = false;
	GetWorldTimerManager().ClearTimer(kkw_charge_tick_timer_handle);
	GetWorldTimerManager().ClearTimer(kkw_charge_end_timer_handle);
	ForceFirstPersonView();
}

void AFPSJanggiAbilityCharacter::RestoreBlinkCharge()
{
	kkw_blink_charges = FMath::Clamp(kkw_blink_charges + 1, 0, 3);
	if (kkw_blink_charges < 3)
	{
		GetWorldTimerManager().SetTimer(kkw_blink_recharge_timer_handle, this, &AFPSJanggiAbilityCharacter::RestoreBlinkCharge, 3.0f, false);
	}
}

void AFPSJanggiAbilityCharacter::BlinkForward()
{
	if (kkw_blink_charges <= 0)
	{
		ShowAbilityText(TEXT("차 blink empty"), FColor::Red);
		ForceFirstPersonView();
		return;
	}

	--kkw_blink_charges;
	const FVector kkw_start = GetActorLocation();
	const FVector kkw_desired_end = kkw_start + GetAimDirection() * 650.0f;

	FHitResult kkw_hit;
	FCollisionQueryParams kkw_params(SCENE_QUERY_STAT(JanggiBlink), false, this);
	GetWorld()->SweepSingleByChannel(kkw_hit, kkw_start, kkw_desired_end, FQuat::Identity, ECC_WorldStatic, GetCapsuleComponent()->GetCollisionShape(), kkw_params);
	SetActorLocation(kkw_hit.bBlockingHit ? kkw_hit.Location : kkw_desired_end, false);

	if (!GetWorldTimerManager().IsTimerActive(kkw_blink_recharge_timer_handle))
	{
		GetWorldTimerManager().SetTimer(kkw_blink_recharge_timer_handle, this, &AFPSJanggiAbilityCharacter::RestoreBlinkCharge, 3.0f, false);
	}

	ShowAbilityText(FString::Printf(TEXT("차 blink (%d/3)"), kkw_blink_charges), FColor::Cyan);
	ForceFirstPersonView();
}

void AFPSJanggiAbilityCharacter::SpawnProjectileShot(const FVector& kkw_direction, float kkw_damage, float kkw_speed, float kkw_blast_radius, const FColor& kkw_debug_color)
{
	if (!kkw_projectile_class)
	{
		return;
	}

	FActorSpawnParameters kkw_spawn_params;
	kkw_spawn_params.Owner = this;
	kkw_spawn_params.Instigator = this;
	kkw_spawn_params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFPSJanggiAbilityProjectile* kkw_projectile = GetWorld()->SpawnActor<AFPSJanggiAbilityProjectile>(kkw_projectile_class, GetAbilityOrigin(145.0f), kkw_direction.Rotation(), kkw_spawn_params);
	if (kkw_projectile)
	{
		kkw_projectile->ConfigureProjectile(kkw_damage, kkw_blast_radius, kkw_debug_color);
		kkw_projectile->FireInDirection(kkw_direction, kkw_speed);
	}
}

void AFPSJanggiAbilityCharacter::PlayCannonJumpBang(const FVector& kkw_origin)
{
	if (kkw_cannon_jump_bang_sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, kkw_cannon_jump_bang_sound, kkw_origin, 1.0f, 0.35f);
	}
}

void AFPSJanggiAbilityCharacter::PlayLoopingAnimation(UAnimSequence* kkw_animation)
{
	if (!kkw_animation || kkw_current_animation == kkw_animation)
	{
		return;
	}

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	GetMesh()->SetAnimation(kkw_animation);
	GetMesh()->Play(true);

	kkw_first_person_mesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	kkw_first_person_mesh->SetAnimation(kkw_animation);
	kkw_first_person_mesh->Play(true);

	kkw_current_animation = kkw_animation;
}

void AFPSJanggiAbilityCharacter::PlayActionAnimation(UAnimSequence* kkw_animation)
{
	if (!kkw_animation)
	{
		return;
	}

	kkw_b_action_animation_active = true;
	GetWorldTimerManager().ClearTimer(kkw_action_animation_timer_handle);

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	GetMesh()->SetAnimation(kkw_animation);
	GetMesh()->Play(false);

	kkw_first_person_mesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	kkw_first_person_mesh->SetAnimation(kkw_animation);
	kkw_first_person_mesh->Play(false);

	kkw_current_animation = kkw_animation;

	const float kkw_action_duration = FMath::Clamp(kkw_animation->GetPlayLength(), 0.15f, 1.1f);
	GetWorldTimerManager().SetTimer(kkw_action_animation_timer_handle, this, &AFPSJanggiAbilityCharacter::EndActionAnimation, kkw_action_duration, false);
}

void AFPSJanggiAbilityCharacter::RefreshMovementAnimation()
{
	const float kkw_speed = GetVelocity().Size2D();
	UAnimSequence* kkw_desired_animation = kkw_idle_animation;

	if (kkw_speed > 260.0f && kkw_run_animation)
	{
		kkw_desired_animation = kkw_run_animation;
	}
	else if (kkw_speed > 8.0f && kkw_move_animation)
	{
		kkw_desired_animation = kkw_move_animation;
	}

	PlayLoopingAnimation(kkw_desired_animation);
}

void AFPSJanggiAbilityCharacter::RefreshFirstPersonMeshTransform()
{
	if (!kkw_first_person_mesh)
	{
		return;
	}

	USkeletalMesh* kkw_mesh_asset = kkw_first_person_mesh->GetSkeletalMeshAsset();
	if (!kkw_mesh_asset)
	{
		return;
	}

	float kkw_view_scale = kkw_first_person_visual_scale;
	FVector kkw_view_center = kkw_first_person_target_center;
	switch (kkw_piece_role)
	{
	case EFPSJanggiPieceRole::Cannon:
		kkw_view_scale = 11.5f;
		kkw_view_center = FVector(112.0f, 34.0f, -38.0f);
		break;
	case EFPSJanggiPieceRole::Guard:
		kkw_view_scale = 5.2f;
		kkw_view_center = FVector(126.0f, 34.0f, -56.0f);
		break;
	case EFPSJanggiPieceRole::Chariot:
		kkw_view_scale = 1.0f;
		kkw_view_center = FVector(0.0f, 0.0f, -1000.0f);
		break;
	}

	const FBoxSphereBounds kkw_bounds = kkw_mesh_asset->GetBounds();
	const FVector kkw_scale(kkw_view_scale);
	const FVector kkw_scaled_origin(
		kkw_bounds.Origin.X * kkw_scale.X,
		kkw_bounds.Origin.Y * kkw_scale.Y,
		kkw_bounds.Origin.Z * kkw_scale.Z);
	const FVector kkw_centering_offset = -kkw_first_person_mesh_rotation.RotateVector(kkw_scaled_origin);

	kkw_first_person_mesh->SetRelativeScale3D(kkw_scale);
	kkw_first_person_mesh->SetRelativeLocation(kkw_view_center + kkw_centering_offset);
	kkw_first_person_mesh->SetRelativeRotation(kkw_first_person_mesh_rotation);
}

void AFPSJanggiAbilityCharacter::RefreshFirstPersonMeshSections()
{
	if (!kkw_first_person_mesh)
	{
		return;
	}

	USkeletalMesh* kkw_mesh_asset = kkw_first_person_mesh->GetSkeletalMeshAsset();
	if (!kkw_mesh_asset)
	{
		return;
	}

	kkw_first_person_mesh->ShowAllMaterialSections(0);
	const TArray<FSkeletalMaterial>& kkw_materials = kkw_mesh_asset->GetMaterials();
	for (int32 kkw_material_index = 0; kkw_material_index < kkw_materials.Num(); ++kkw_material_index)
	{
		const FString kkw_slot_name = kkw_materials[kkw_material_index].MaterialSlotName.ToString().ToLower();
		bool kkw_b_is_first_person_part = false;
		switch (kkw_piece_role)
		{
		case EFPSJanggiPieceRole::Cannon:
			kkw_b_is_first_person_part = kkw_slot_name.Contains(TEXT("canon"));
			break;
		case EFPSJanggiPieceRole::Guard:
			kkw_b_is_first_person_part = kkw_slot_name.Contains(TEXT("shield"));
			break;
		case EFPSJanggiPieceRole::Chariot:
			kkw_b_is_first_person_part = false;
			break;
		}

		kkw_first_person_mesh->ShowMaterialSection(kkw_material_index, kkw_material_index, kkw_b_is_first_person_part, 0);
	}
}

void AFPSJanggiAbilityCharacter::RefreshFistViewmodelTransform()
{
	if (!kkw_left_fist_mesh || !kkw_right_fist_mesh)
	{
		return;
	}

	kkw_left_fist_mesh->SetRelativeLocation(FVector(94.0f, -28.0f, -38.0f));
	kkw_left_fist_mesh->SetRelativeRotation(FRotator(-8.0f, 8.0f, -10.0f));
	kkw_left_fist_mesh->SetRelativeScale3D(FVector(0.28f, 0.16f, 0.15f));

	kkw_right_fist_mesh->SetRelativeLocation(FVector(94.0f, 28.0f, -38.0f));
	kkw_right_fist_mesh->SetRelativeRotation(FRotator(-8.0f, -8.0f, 10.0f));
	kkw_right_fist_mesh->SetRelativeScale3D(FVector(0.28f, 0.16f, 0.15f));
}

void AFPSJanggiAbilityCharacter::SetFistViewmodelVisible(bool kkw_b_visible)
{
	if (kkw_left_fist_mesh)
	{
		kkw_left_fist_mesh->SetVisibility(kkw_b_visible, true);
	}

	if (kkw_right_fist_mesh)
	{
		kkw_right_fist_mesh->SetVisibility(kkw_b_visible, true);
	}
}

void AFPSJanggiAbilityCharacter::ApplyCameraMode()
{
	const bool kkw_b_show_first_person_mesh = kkw_b_selected_view_target && !kkw_b_third_person_active;
	const bool kkw_b_show_fist_viewmodel = kkw_b_show_first_person_mesh && kkw_piece_role == EFPSJanggiPieceRole::Chariot;
	GetMesh()->SetVisibility(!kkw_b_show_first_person_mesh, true);

	if (kkw_first_person_mesh)
	{
		kkw_first_person_mesh->SetVisibility(kkw_b_show_first_person_mesh && !kkw_b_show_fist_viewmodel, true);
		if (kkw_b_show_first_person_mesh && !kkw_b_show_fist_viewmodel)
		{
			RefreshFirstPersonMeshSections();
			RefreshFirstPersonMeshTransform();
		}
	}

	SetFistViewmodelVisible(kkw_b_show_fist_viewmodel);
	if (kkw_b_show_fist_viewmodel)
	{
		RefreshFistViewmodelTransform();
	}

	if (kkw_follow_camera)
	{
		kkw_follow_camera->SetRelativeLocation(kkw_b_third_person_active ? kkw_third_person_camera_location : kkw_first_person_camera_location);
		kkw_follow_camera->SetFieldOfView(kkw_b_third_person_active ? 82.0f : 90.0f);
	}
}

void AFPSJanggiAbilityCharacter::EndActionAnimation()
{
	kkw_b_action_animation_active = false;
	RefreshMovementAnimation();
	ForceFirstPersonView();
}
