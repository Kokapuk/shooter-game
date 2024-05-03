#pragma once

#include "CoreMinimal.h"
#include "SGPlayerState.h"
#include "SGWeaponComponent.h"
#include "GameFramework/Character.h"
#include "SGCharacter.generated.h"

class USGWeaponComponent;
class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;

UCLASS(config=Game)
class ASGCharacter : public ACharacter
{
	GENERATED_BODY()

	DECLARE_DELEGATE_OneParam(FCrouchDelegate, bool);

public:
	ASGCharacter();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	                         AActor* DamageCauser) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	UFUNCTION(BlueprintPure)
	USkeletalMeshComponent* GetArmsMesh() const { return ArmsMesh; }

	UFUNCTION(BlueprintPure)
	USkeletalMeshComponent* GetFirstPersonWeaponMesh() const { return FirstPersonWeaponMesh; }

	UFUNCTION(BlueprintPure)
	USkeletalMeshComponent* GetThirdPersonWeaponMesh() const { return ThirdPersonWeaponMesh; }

	UFUNCTION(BlueprintPure)
	UCameraComponent* GetCamera() const { return Camera; }

	UFUNCTION(BlueprintPure)
	USGWeaponComponent* GetWeaponComponent() const { return Weapon; }

	UFUNCTION(BlueprintPure)
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure)
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure)
	ETeam GetTeam() const;

	UFUNCTION(NetMulticast, Unreliable)
	void MultiPlayHitReactMontage(const FName& HitBoneName);

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UCameraComponent* Camera;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	USkeletalMeshComponent* ArmsMesh;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	USkeletalMeshComponent* FirstPersonWeaponMesh;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	USkeletalMeshComponent* ThirdPersonWeaponMesh;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	USGWeaponComponent* Weapon;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float MaxHealth;

	UPROPERTY(Replicated, BlueprintReadOnly, VisibleInstanceOnly)
	float Health;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* HitReactMontage;

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	void MoveForward(const float Value) { AddMovementInput(GetActorForwardVector(), Value); };
	void MoveRight(const float Value) { AddMovementInput(GetActorRightVector(), Value); };

	UFUNCTION(BlueprintCallable)
	void Fire();

	UFUNCTION(BlueprintCallable)
	void Reload();

private:
	float TargetCameraHeight;
};
