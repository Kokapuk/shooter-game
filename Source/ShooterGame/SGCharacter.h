#pragma once

#include "CoreMinimal.h"
#include "Delegates.h"
#include "SGWeaponComponent.h"
#include "GameFramework/Character.h"
#include "SGCharacter.generated.h"

class USGAbilityDataAsset;
class USGAbilityComponent;
class USGBlindnessComponent;
class USGWeaponComponent;
class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;

DECLARE_DELEGATE_OneParam(FCrouchDelegate, bool);

UCLASS()
class ASGCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASGCharacter(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable)
	FOnDie OnDie;

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

	template <class T>
	T* GetWeaponComponent() const { return Cast<T>(Weapon); }

	UFUNCTION(BlueprintPure)
	USGBlindnessComponent* GetBlindnessComponent() const { return BlindnessComponent; }

	UFUNCTION(Server, Reliable, BlueprintCallable, DisplayName="Set Ability")
	void ServerSetAbility(const USGAbilityDataAsset* NewAbility);

	UFUNCTION(BlueprintPure)
	USGAbilityComponent* GetAbilityComponent() const { return AbilityComponent; }

	template <class T>
	T* GetAbilityComponent() const { return Cast<T>(AbilityComponent); }

	UFUNCTION(BlueprintPure)
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure)
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure)
	bool IsDead() const { return bIsDead; }

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

	UPROPERTY(EditDefaultsOnly)
	USGBlindnessComponent* BlindnessComponent;

	UPROPERTY(Replicated)
	USGAbilityComponent* AbilityComponent;

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

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	void MoveForward(const float Value) { AddMovementInput(GetActorForwardVector(), Value); };
	void MoveRight(const float Value) { AddMovementInput(GetActorRightVector(), Value); };

	UFUNCTION(BlueprintCallable)
	void FireWeapon() { Weapon->CosmeticFire(); }

	UFUNCTION(BlueprintCallable)
	void ReloadWeapon() { Weapon->CosmeticReload(); }

	UFUNCTION(BlueprintCallable)
	void UtilizeAbility();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Die")
	void AuthDie(APawn* Killer, const bool bIsHeadshot);

	UFUNCTION(NetMulticast, Reliable)
	void MultiDie(APawn* Killer, const bool bIsHeadshot);

	UFUNCTION(NetMulticast, Reliable)
	void MultiReset();

	UFUNCTION(BlueprintCallable)
	void ResetAnimations() const;

	UFUNCTION(BlueprintCallable)
	void SetDeadCollision(const bool bNewDeadCollision) const;

private:
	float TargetCameraHeight;

	UFUNCTION()
	void OnRep_IsDead();
};
