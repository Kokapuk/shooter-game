#pragma once

#include "CoreMinimal.h"
#include "SGGameState.h"
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDie);

UCLASS(config=Game)
class ASGCharacter : public ACharacter
{
	GENERATED_BODY()

	DECLARE_DELEGATE_OneParam(FCrouchDelegate, bool);

public:
	ASGCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
	                              AActor* DamageCauser) const override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	                         AActor* DamageCauser) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	UPROPERTY(BlueprintAssignable)
	FOnCharacterDie OnDie;

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
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure)
	ETeam GetTeam() const;

	UFUNCTION(NetMulticast, Unreliable)
	void MultiPlayHitReactMontage(const FName& HitBoneName);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Reset")
	void AuthReset(const AActor* PlayerStart);

protected:
	UPROPERTY(EditDefaultsOnly)
	UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly)
	USkeletalMeshComponent* ArmsMesh;

	UPROPERTY(EditDefaultsOnly)
	USkeletalMeshComponent* FirstPersonWeaponMesh;

	UPROPERTY(EditDefaultsOnly)
	USkeletalMeshComponent* ThirdPersonWeaponMesh;

	UPROPERTY(EditDefaultsOnly)
	USGWeaponComponent* Weapon;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=1.f))
	float MaxHealth;

	UPROPERTY(Replicated, VisibleInstanceOnly)
	float Health;

	UPROPERTY(ReplicatedUsing=OnRep_IsDead, VisibleInstanceOnly)
	uint8 bIsDead : 1;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInstance* RedTeamMaterial;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* BlueTeamMaterial;

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	void MoveForward(const float Value) { AddMovementInput(GetActorForwardVector(), Value); };
	void MoveRight(const float Value) { AddMovementInput(GetActorRightVector(), Value); };

	UFUNCTION(BlueprintCallable)
	void Fire() { Weapon->CosmeticFire(); }

	UFUNCTION(BlueprintCallable)
	void Reload() { Weapon->ServerReload(); }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Die")
	void AuthDie();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, DisplayName="Reset Animations")
	void MultiResetAnimations();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void MultiSetDeadCollision(const bool bNewDeadCollision);

private:
	float TargetCameraHeight;

	UFUNCTION()
	void HandleMatchBegin();

	UFUNCTION()
	void OnRep_IsDead();
};
