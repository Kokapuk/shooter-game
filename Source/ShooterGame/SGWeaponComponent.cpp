#include "SGWeaponComponent.h"

#include "SGCharacter.h"
#include "SGWeaponDataAsset.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

USGWeaponComponent::USGWeaponComponent()
{
	Equipped = nullptr;
	SetIsReplicatedByDefault(true);
}

void USGWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USGWeaponComponent, Equipped);
}

void USGWeaponComponent::ServerEquip_Implementation(USGWeaponDataAsset* Weapon)
{
	Equipped = Weapon;
	OnRep_Equipped();
}

void USGWeaponComponent::ServerFire_Implementation()
{
	if (Equipped == nullptr) return;

	ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	if (!IsValid(OwningCharacter)) return;

	const UCameraComponent* Camera = OwningCharacter->GetCamera();
	if (Camera == nullptr) return;

	const FVector End = Camera->GetComponentLocation() + (UKismetMathLibrary::GetForwardVector(
		OwningCharacter->GetBaseAimRotation()) * 10000.f);

	FHitResult HitResult;

	const bool bHit = UKismetSystemLibrary::LineTraceSingle(OwningCharacter, Camera->GetComponentLocation(), End,
	                                                        UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1),
	                                                        false,
	                                                        {}, EDrawDebugTrace::ForDuration, HitResult, true,
	                                                        FLinearColor::Red, FLinearColor::Green, .5f);

	MultiFire(HitResult);

	if (!bHit) return;

	AActor* HitActor = HitResult.GetActor();
	if (!IsValid(HitActor) || !HitActor->CanBeDamaged()) return;

	HitActor->TakeDamage(HitResult.BoneName == "head" ? Equipped->HeadShotDamage : Equipped->BodyShotDamage,
	                     FDamageEvent(), OwningCharacter->GetController(), OwningCharacter);
}

void USGWeaponComponent::MultiFire_Implementation(const FHitResult& HitResult)
{
	PlayFireAnimations();
	SpawnTracer(HitResult);
	SpawnImpactParticles(HitResult);
}

void USGWeaponComponent::ServerReload_Implementation()
{
	if (Equipped == nullptr) return;

	MultiReload();
}

void USGWeaponComponent::MultiReload_Implementation()
{
	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());

	if (OwningCharacter && OwningCharacter->IsLocallyControlled())
	{
		UAnimInstance* WeaponAnimInstance = OwningCharacter->GetFirstPersonWeaponMesh()->GetAnimInstance();

		if (IsValid(WeaponAnimInstance) && IsValid(Equipped->WeaponReloadMontage))
		{
			WeaponAnimInstance->Montage_Play(Equipped->WeaponReloadMontage,
			                                 Equipped->WeaponReloadMontage->GetPlayLength() / Equipped->ReloadTime);
		}
	}
	else if (!OwningCharacter->IsLocallyControlled())
	{
		UAnimInstance* BodyAnimInstance = OwningCharacter->GetMesh()->GetAnimInstance();
		UAnimInstance* WeaponAnimInstance = OwningCharacter->GetThirdPersonWeaponMesh()->GetAnimInstance();

		if (IsValid(BodyAnimInstance) && IsValid(Equipped->ThirdPersonReloadMontage))
		{
			BodyAnimInstance->Montage_Play(Equipped->ThirdPersonReloadMontage,
			                               Equipped->ThirdPersonReloadMontage->GetPlayLength() / Equipped->ReloadTime);
		}

		if (IsValid(WeaponAnimInstance) && IsValid(Equipped->WeaponReloadMontage))
		{
			WeaponAnimInstance->Montage_Play(Equipped->WeaponReloadMontage,
			                                 Equipped->WeaponReloadMontage->GetPlayLength() / Equipped->ReloadTime);
		}
	}
}

void USGWeaponComponent::PlayFireAnimations() const
{
	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	
	if (OwningCharacter && OwningCharacter->IsLocallyControlled())
	{
		UAnimInstance* ArmsAnimInstance = OwningCharacter->GetArmsMesh()->GetAnimInstance();
		UAnimInstance* WeaponAnimInstance = OwningCharacter->GetFirstPersonWeaponMesh()->GetAnimInstance();

		if (IsValid(ArmsAnimInstance) && IsValid(Equipped->FirstPersonFireMontage))
		{
			ArmsAnimInstance->Montage_Play(Equipped->FirstPersonFireMontage);
		}

		if (IsValid(WeaponAnimInstance) && IsValid(Equipped->WeaponFireMontage))
		{
			WeaponAnimInstance->Montage_Play(Equipped->WeaponFireMontage);
		}
	}
	else if (!OwningCharacter->IsLocallyControlled())
	{
		UAnimInstance* WeaponAnimInstance = OwningCharacter->GetThirdPersonWeaponMesh()->GetAnimInstance();

		if (IsValid(WeaponAnimInstance) && IsValid(Equipped->WeaponFireMontage))
		{
			WeaponAnimInstance->Montage_Play(Equipped->WeaponFireMontage);
		}
	}
}

void USGWeaponComponent::SpawnTracer(const FHitResult& HitResult) const
{
	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	const AActor* HitActor = HitResult.GetActor();
	
	if (OwningCharacter && OwningCharacter->IsLocallyControlled())
	{
		const USkeletalMeshComponent* WeaponMesh = OwningCharacter->GetFirstPersonWeaponMesh();

		if (IsValid(WeaponMesh))
		{
			GetWorld()->SpawnActor<AActor>(Equipped->TracerClass, WeaponMesh->GetSocketLocation("MuzzleFlash"),
										   UKismetMathLibrary::FindLookAtRotation(
											   WeaponMesh->GetComponentLocation(),
											   IsValid(HitResult.Actor.Get())
												   ? HitResult.Location
												   : HitResult.TraceEnd));
		}
	}
	else if (!OwningCharacter->IsLocallyControlled())
	{
		const USkeletalMeshComponent* WeaponMesh = OwningCharacter->GetThirdPersonWeaponMesh();

		if (IsValid(WeaponMesh))
		{
			GetWorld()->SpawnActor<AActor>(Equipped->TracerClass, WeaponMesh->GetSocketLocation("MuzzleFlash"),
										   UKismetMathLibrary::FindLookAtRotation(
											   WeaponMesh->GetComponentLocation(),
											   IsValid(HitActor)
												   ? HitResult.Location
												   : HitResult.TraceEnd));
		}
	}
}

void USGWeaponComponent::SpawnImpactParticles(const FHitResult& HitResult) const
{
	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	const AActor* HitActor = HitResult.GetActor();
	
	if (IsValid(HitActor) && IsValid(Equipped->BodyImpactCue) && IsValid(Equipped->SurfaceImpactCue))
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
	if (!IsValid(OwningCharacter)) return;

	if (OwningCharacter->IsLocallyControlled())
	{
		return OwningCharacter->GetFirstPersonWeaponMesh()->SetSkeletalMesh(
			Equipped == nullptr ? nullptr : Equipped->Mesh);
	}
	else
	{
		return OwningCharacter->GetThirdPersonWeaponMesh()->SetSkeletalMesh(
			Equipped == nullptr ? nullptr : Equipped->Mesh);
	}
}
