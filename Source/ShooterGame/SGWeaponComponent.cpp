#include "SGWeaponComponent.h"

#include "SGCharacter.h"
#include "SGWeaponDataAsset.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

USGWeaponComponent::USGWeaponComponent()
{
	Equipped = nullptr;

	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
}

void USGWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USGWeaponComponent, Equipped);
}

void USGWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwner()->HasAuthority() && IsValid(Equipped) && !bIsAutomaticallyFiring)
	{
		ShootingError = FMath::Clamp(
			ShootingError - (Equipped->MaxSprayShootingError / Equipped->MaxShootingErrorRecoveryTime) * DeltaTime,
			0.f, Equipped->MaxSprayShootingError);
	}

	if (GetOwner()->HasAuthority() && bIsAutomaticallyFiring && IsValid(Equipped) && TimeToFire == 0.f)
	{
		AuthFire();
	}
	else if (IsValid(Equipped) && TimeToFire > 0.f)
	{
		TimeToFire = FMath::Clamp(TimeToFire - DeltaTime, 0.f, Equipped->TimeBetweenShots);
	}
}

void USGWeaponComponent::ServerEquip_Implementation(USGWeaponDataAsset* Weapon)
{
	bIsAutomaticallyFiring = false;
	TimeToFire = 0.f;
	ShootingError = 0.f;
	Equipped = Weapon;
	OnRep_Equipped();
}

void USGWeaponComponent::ServerStartFire_Implementation()
{
	if (!IsValid(Equipped)) return;

	if (Equipped->bIsAutomatic) bIsAutomaticallyFiring = true;
	else if (TimeToFire == 0.f) AuthFire();
}

void USGWeaponComponent::ServerStopFire_Implementation()
{
	bIsAutomaticallyFiring = false;
}

void USGWeaponComponent::AuthFire()
{
	if (!GetOwner()->HasAuthority() || !IsValid(Equipped) || TimeToFire > 0.f) return;

	TimeToFire = Equipped->TimeBetweenShots;

	ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	const UCameraComponent* Camera = OwningCharacter->GetCamera();
	check(IsValid(Camera))

	const FVector End = Camera->GetComponentLocation() + GetFireDirection() * 10000.f;

	FHitResult HitResult;
	const bool bHit = UKismetSystemLibrary::LineTraceSingle(OwningCharacter, Camera->GetComponentLocation(), End,
	                                                        UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1),
	                                                        false,
	                                                        {}, EDrawDebugTrace::ForDuration, HitResult, true,
	                                                        FLinearColor::Red, FLinearColor::Green, .5f);

	MultiFire(HitResult);

	ShootingError = FMath::Clamp(ShootingError + Equipped->MaxSprayShootingError / 5, 0.f,
	                             Equipped->MaxSprayShootingError);

	if (!bHit) return;

	AActor* HitActor = HitResult.GetActor();
	if (!IsValid(HitActor) || !HitActor->CanBeDamaged()) return;

	ASGCharacter* HitCharacter = Cast<ASGCharacter>(HitActor);

	if (IsValid(HitCharacter))
	{
		HitCharacter->MultiPlayHitReactMontage(HitResult.BoneName);
	}

	HitActor->TakeDamage(HitResult.BoneName == "head" ? Equipped->HeadShotDamage : Equipped->BodyShotDamage,
	                     FDamageEvent(), OwningCharacter->GetController(), OwningCharacter);
}

FVector USGWeaponComponent::GetFireDirection() const
{
	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	const FRotator AimRotation = OwningCharacter->GetBaseAimRotation();
	const float MovingShootingError = OwningCharacter->GetVelocity().Size() / OwningCharacter->GetCharacterMovement()->
		MaxWalkSpeed * Equipped->MaxMovingShootingError;

	FVector AimDirection = UKismetMathLibrary::GetForwardVector(AimRotation);
	const float RandomHorizontalAngle = FMath::FRandRange(-ShootingError - MovingShootingError,
	                                                      ShootingError + MovingShootingError);
	const float RandomVerticalAngle = FMath::FRandRange(-ShootingError - MovingShootingError,
	                                                    ShootingError + MovingShootingError);
	AimDirection = AimDirection.RotateAngleAxis(RandomHorizontalAngle, UKismetMathLibrary::GetUpVector(AimRotation));
	AimDirection = AimDirection.RotateAngleAxis(RandomVerticalAngle, UKismetMathLibrary::GetRightVector(AimRotation));

	return AimDirection;
}

void USGWeaponComponent::MultiFire_Implementation(const FHitResult& HitResult)
{
	PlayFireAnimations();
	SpawnTracer(HitResult);
	SpawnImpactParticles(HitResult);
}

void USGWeaponComponent::ServerReload_Implementation()
{
	if (!IsValid(Equipped)) return;

	MultiReload();
}

void USGWeaponComponent::MultiReload_Implementation()
{
	check(IsValid(Equipped->WeaponReloadMontage))
	check(IsValid(Equipped->ThirdPersonReloadMontage))

	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	if (OwningCharacter->IsLocallyControlled())
	{
		UAnimInstance* WeaponAnimInstance = OwningCharacter->GetFirstPersonWeaponMesh()->GetAnimInstance();
		check(IsValid(WeaponAnimInstance))

		WeaponAnimInstance->Montage_Play(Equipped->WeaponReloadMontage,
		                                 Equipped->WeaponReloadMontage->GetPlayLength() / Equipped->ReloadTime);
	}
	else if (!OwningCharacter->IsLocallyControlled())
	{
		UAnimInstance* BodyAnimInstance = OwningCharacter->GetMesh()->GetAnimInstance();
		UAnimInstance* WeaponAnimInstance = OwningCharacter->GetThirdPersonWeaponMesh()->GetAnimInstance();
		check(IsValid(BodyAnimInstance))
		check(IsValid(WeaponAnimInstance))

		BodyAnimInstance->Montage_Play(Equipped->ThirdPersonReloadMontage,
		                               Equipped->ThirdPersonReloadMontage->GetPlayLength() / Equipped->ReloadTime);
		WeaponAnimInstance->Montage_Play(Equipped->WeaponReloadMontage,
		                                 Equipped->WeaponReloadMontage->GetPlayLength() / Equipped->ReloadTime);
	}
}

void USGWeaponComponent::PlayFireAnimations() const
{
	check(IsValid(Equipped->FirstPersonFireMontage))
	check(IsValid(Equipped->WeaponFireMontage))

	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	if (OwningCharacter->IsLocallyControlled())
	{
		UAnimInstance* ArmsAnimInstance = OwningCharacter->GetArmsMesh()->GetAnimInstance();
		UAnimInstance* WeaponAnimInstance = OwningCharacter->GetFirstPersonWeaponMesh()->GetAnimInstance();
		check(IsValid(ArmsAnimInstance))
		check(IsValid(WeaponAnimInstance))

		ArmsAnimInstance->Montage_Play(Equipped->FirstPersonFireMontage);
		WeaponAnimInstance->Montage_Play(Equipped->WeaponFireMontage);
	}
	else if (!OwningCharacter->IsLocallyControlled())
	{
		UAnimInstance* WeaponAnimInstance = OwningCharacter->GetThirdPersonWeaponMesh()->GetAnimInstance();
		check(IsValid(WeaponAnimInstance))

		WeaponAnimInstance->Montage_Play(Equipped->WeaponFireMontage);
	}
}

void USGWeaponComponent::SpawnTracer(const FHitResult& HitResult) const
{
	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	const AActor* HitActor = HitResult.GetActor();

	if (OwningCharacter->IsLocallyControlled())
	{
		const USkeletalMeshComponent* WeaponMesh = OwningCharacter->GetFirstPersonWeaponMesh();
		check(IsValid(WeaponMesh))

		GetWorld()->SpawnActor<AActor>(Equipped->TracerClass, WeaponMesh->GetSocketLocation("MuzzleFlash"),
		                               UKismetMathLibrary::FindLookAtRotation(
			                               WeaponMesh->GetComponentLocation(),
			                               IsValid(HitResult.Actor.Get())
				                               ? HitResult.Location
				                               : HitResult.TraceEnd));
	}
	else if (!OwningCharacter->IsLocallyControlled())
	{
		const USkeletalMeshComponent* WeaponMesh = OwningCharacter->GetThirdPersonWeaponMesh();
		check(IsValid(WeaponMesh))

		GetWorld()->SpawnActor<AActor>(Equipped->TracerClass, WeaponMesh->GetSocketLocation("MuzzleFlash"),
		                               UKismetMathLibrary::FindLookAtRotation(
			                               WeaponMesh->GetComponentLocation(),
			                               IsValid(HitActor)
				                               ? HitResult.Location
				                               : HitResult.TraceEnd));
	}
}

void USGWeaponComponent::SpawnImpactParticles(const FHitResult& HitResult) const
{
	check(IsValid(Equipped->BodyImpactCue))
	check(IsValid(Equipped->SurfaceImpactCue))

	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(OwningCharacter)

	const AActor* HitActor = HitResult.GetActor();

	if (IsValid(HitActor))
	{
		UGameplayStatics::PlaySoundAtLocation(OwningCharacter,
		                                      HitActor->CanBeDamaged()
			                                      ? Equipped->BodyImpactCue
			                                      : Equipped->SurfaceImpactCue, HitResult.Location);

		UParticleSystem* ImpactParticles = nullptr;

		if (HitActor->CanBeDamaged())
		{
			ImpactParticles = HitResult.BoneName == "head" ? Equipped->HeadShotParticles : Equipped->BodyShotParticles;
		}
		else ImpactParticles = Equipped->SurfaceImpactParticles;

		UGameplayStatics::SpawnEmitterAtLocation(OwningCharacter, ImpactParticles, HitResult.Location,
		                                         HitResult.Normal.Rotation());
	}
}

void USGWeaponComponent::OnRep_Equipped() const
{
	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(OwningCharacter)

	if (OwningCharacter->IsLocallyControlled())
	{
		OwningCharacter->GetFirstPersonWeaponMesh()->SetSkeletalMesh(
			Equipped == nullptr ? nullptr : Equipped->Mesh);
	}
	else
	{
		OwningCharacter->GetThirdPersonWeaponMesh()->SetSkeletalMesh(
			Equipped == nullptr ? nullptr : Equipped->Mesh);
	}
}
