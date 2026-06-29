// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Camera/CameraTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSJanggiAbilityCharacter.generated.h"

class AFPSJanggiAbilityProjectile;
class UAnimSequence;
class UCameraComponent;
class USoundBase;
class USpringArmComponent;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EFPSJanggiPieceRole : uint8
{
	Cannon,
	Guard,
	Chariot
};

UCLASS()
class FPSJANGGI_API AFPSJanggiAbilityCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFPSJanggiAbilityCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* kkw_player_input_component) override;
	virtual void Tick(float kkw_delta_seconds) override;
	virtual float TakeDamage(float kkw_damage_amount, FDamageEvent const& kkw_damage_event, AController* kkw_event_instigator, AActor* kkw_damage_causer) override;
	virtual void CalcCamera(float kkw_delta_time, FMinimalViewInfo& kkw_out_result) override;

	void ConfigurePiece(EFPSJanggiPieceRole kkw_in_role, USkeletalMesh* kkw_in_mesh, const FTransform& kkw_source_mesh_transform, bool kkw_b_use_source_placement);
	void ConfigureAnimations(UAnimSequence* kkw_idle_animation, UAnimSequence* kkw_move_animation, UAnimSequence* kkw_attack_animation, UAnimSequence* kkw_run_animation);
	EFPSJanggiPieceRole GetPieceRole() const { return kkw_piece_role; }
	void ForceFirstPersonView();
	void SetFirstPersonActive(bool kkw_b_active);
	void ToggleCameraMode();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> kkw_camera_boom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> kkw_follow_camera;

	UPROPERTY(VisibleAnywhere, Category = "kkw_viewmodel")
	TObjectPtr<USkeletalMeshComponent> kkw_first_person_mesh;

	UPROPERTY(EditDefaultsOnly, Category = "kkw_viewmodel")
	float kkw_first_person_visual_scale = 5.5f;

	UPROPERTY(EditDefaultsOnly, Category = "kkw_viewmodel")
	FVector kkw_first_person_target_center = FVector(78.0f, 0.0f, -24.0f);

	UPROPERTY(EditDefaultsOnly, Category = "kkw_viewmodel")
	FRotator kkw_first_person_mesh_rotation = FRotator(0.0f, -90.0f, 0.0f);

	UPROPERTY(VisibleAnywhere, Category = "kkw_viewmodel")
	FVector kkw_first_person_camera_location = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "kkw_viewmodel")
	FVector kkw_third_person_camera_location = FVector(-320.0f, 0.0f, 120.0f);

	UPROPERTY(EditDefaultsOnly, Category = "kkw_piece")
	EFPSJanggiPieceRole kkw_piece_role = EFPSJanggiPieceRole::Cannon;

	UPROPERTY(EditDefaultsOnly, Category = "kkw_piece")
	FText kkw_piece_label;

	UPROPERTY(EditDefaultsOnly, Category = "kkw_piece")
	float kkw_character_visual_scale = 1.6f;

	UPROPERTY(EditDefaultsOnly, Category = "kkw_piece")
	float kkw_piece_forward_yaw_offset = -90.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float kkw_max_health = 400.0f;

	UPROPERTY(VisibleAnywhere, Category = "Stats")
	float kkw_health = 400.0f;

	UPROPERTY(VisibleAnywhere, Category = "Stats")
	float kkw_damage_taken_total = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float kkw_guard_damage_reduction = 0.65f;

	UPROPERTY(EditDefaultsOnly, Category = "Cannon")
	TSubclassOf<AFPSJanggiAbilityProjectile> kkw_projectile_class;

	UPROPERTY(EditDefaultsOnly, Category = "Cannon")
	TObjectPtr<USoundBase> kkw_cannon_jump_bang_sound;

	UPROPERTY(VisibleAnywhere, Category = "kkw_animation")
	TObjectPtr<UAnimSequence> kkw_idle_animation;

	UPROPERTY(VisibleAnywhere, Category = "kkw_animation")
	TObjectPtr<UAnimSequence> kkw_move_animation;

	UPROPERTY(VisibleAnywhere, Category = "kkw_animation")
	TObjectPtr<UAnimSequence> kkw_attack_animation;

	UPROPERTY(VisibleAnywhere, Category = "kkw_animation")
	TObjectPtr<UAnimSequence> kkw_run_animation;

	UPROPERTY(VisibleAnywhere, Category = "kkw_animation")
	TObjectPtr<UAnimSequence> kkw_current_animation;

	bool kkw_b_parrying = false;
	bool kkw_b_guarding = false;
	bool kkw_b_charging = false;
	bool kkw_b_action_animation_active = false;
	bool kkw_b_selected_view_target = false;
	bool kkw_b_third_person_active = false;
	int32 kkw_blink_charges = 3;

	FTimerHandle kkw_parry_timer_handle;
	FTimerHandle kkw_charge_tick_timer_handle;
	FTimerHandle kkw_charge_end_timer_handle;
	FTimerHandle kkw_blink_recharge_timer_handle;
	FTimerHandle kkw_action_animation_timer_handle;

	void MoveForward(float kkw_value);
	void MoveRight(float kkw_value);
	void Turn(float kkw_value);
	void LookUp(float kkw_value);

	void PrimaryPressed();
	void SecondaryPressed();
	void SecondaryReleased();
	void AbilityEPressed();
	void AbilityQPressed();

	void CenterMeshOnCapsule();
	void ApplyRoleStats();
	FVector GetAimDirection() const;
	FVector GetAbilityOrigin(float kkw_forward_offset = 120.0f) const;
	void ShowAbilityText(const FString& kkw_message, const FColor& kkw_color) const;
	void DealMeleeDamage(float kkw_damage, float kkw_range, float kkw_radius, float kkw_knockback, const FColor& kkw_debug_color, const FString& kkw_debug_label);
	void DealRadialAbilityDamage(float kkw_damage, float kkw_radius, const FVector& kkw_origin, const FColor& kkw_debug_color, const FString& kkw_debug_label);

	void EndParry();
	void ChargeTick();
	void EndCharge();
	void RestoreBlinkCharge();
	void BlinkForward();

	void SpawnProjectileShot(const FVector& kkw_direction, float kkw_damage, float kkw_speed, float kkw_blast_radius, const FColor& kkw_debug_color);
	void PlayCannonJumpBang(const FVector& kkw_origin);
	void PlayLoopingAnimation(UAnimSequence* kkw_animation);
	void PlayActionAnimation(UAnimSequence* kkw_animation);
	void RefreshMovementAnimation();
	void RefreshFirstPersonMeshTransform();
	void RefreshFirstPersonMeshSections();
	void ApplyCameraMode();
	void EndActionAnimation();
};
