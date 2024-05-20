#include "SGWeaponComponent.h"

#include "SGCharacter.h"
#include "SGGameUserSettings.h"
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
	DOREPLIFETIME_CONDITION(USGWeaponComponent, Rounds, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(USGWeaponComponent, bIsReloading, COND_OwnerOnly);
}

void USGWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if ((HasAuthority() || IsLocallyControlled()) && IsValid(Equipped) && TimeToFire > 0.f)
	{
		TimeToFire = FMath::Clamp(TimeToFire - DeltaTime, 0.f, Equipped->TimeBetweenShots);
	}
}

bool USGWeaponComponent::IsLocallyControlled() const
{
	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	return OwningCharacter->IsLocallyControlled();
}

void USGWeaponComponent::ServerEquip_Implementation(USGWeaponDataAsset* Weapon)
{
	GetOwner()->GetWorldTimerManager().ClearTimer(ReloadingHandle);
	bIsReloading = false;
	TimeToFire = 0.f;

	Equipped = Weapon;
	OnRep_Equipped();
	Rounds = IsValid(Weapon) ? Weapon->MagazineCapacity : 0;
}

bool USGWeaponComponent::CanFire() const
{
	return (HasAuthority() || IsLocallyControlled()) && IsValid(Equipped) && Rounds > 0 && TimeToFire == 0.f && !
			bIsReloading;
}

void USGWeaponComponent::CosmeticFire()
{
	if (!IsLocallyControlled()) return;
	if (!Rounds) return ServerReload();
	if (!CanFire()) return;

	if (!HasAuthority()) TimeToFire = Equipped->TimeBetweenShots;
	PlayFireAnimations();

	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	const UCameraComponent* Camera = OwningCharacter->GetCamera();
	check(IsValid(Camera))

	const FVector End = Camera->GetComponentLocation() + GetFireDirection() * 35000.f;

	FHitResult HitResult;
	UKismetSystemLibrary::LineTraceSingle(OwningCharacter, Camera->GetComponentLocation(), End,
															UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1),
															false,
															{}, EDrawDebugTrace::ForDuration, HitResult, true,
															FLinearColor::Red, FLinearColor::Green, .5f);

	ServerFire(HitResult);
}

void USGWeaponComponent::ServerFire_Implementation(const FHitResult& HitResult)
{
	if (!CanFire()) return;

	TimeToFire = Equipped->TimeBetweenShots;
	Rounds--;

	MultiFire(HitResult);

	if (!HitResult.bBlockingHit) return;

	AActor* HitActor = HitResult.GetActor();
	if (!IsValid(HitActor) || !HitActor->CanBeDamaged()) return;

	ASGCharacter* HitCharacter = Cast<ASGCharacter>(HitActor);

	if (IsValid(HitCharacter))
	{
		HitCharacter->MultiPlayHitReactMontage(HitResult.BoneName);
	}

	ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(IsValid(OwningCharacter))
	
	HitActor->TakeDamage(HitResult.BoneName == "head" ? Equipped->HeadShotDamage : Equipped->BodyShotDamage,
	                     FDamageEvent(), OwningCharacter->GetController(), OwningCharacter);
}

FVector USGWeaponComponent::GetFireDirection() const
{
	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	const FRotator AimRotation = OwningCharacter->GetBaseAimRotation();
	const float MovingShootingError = OwningCharacter->GetVelocity().Size() / OwningCharacter->GetCharacterMovement()->
		MaxWalkSpeed * Equipped->MaxShootingError;

	FVector AimDirection = UKismetMathLibrary::GetForwardVector(AimRotation);
	const float RandomHorizontalAngle = FMath::FRandRange(-MovingShootingError, MovingShootingError);
	const float RandomVerticalAngle = FMath::FRandRange(-MovingShootingError, MovingShootingError);
	AimDirection = AimDirection.RotateAngleAxis(RandomHorizontalAngle, UKismetMathLibrary::GetUpVector(AimRotation));
	AimDirection = AimDirection.RotateAngleAxis(RandomVerticalAngle, UKismetMathLibrary::GetRightVector(AimRotation));

	return AimDirection;
}

void USGWeaponComponent::AuthReset()
{
	if (!HasAuthority()) return;

	GetOwner()->GetWorldTimerManager().ClearTimer(ReloadingHandle);
	bIsReloading = false;
	Rounds = IsValid(Equipped) ? Equipped->MagazineCapacity : 0;
}

void USGWeaponComponent::MultiFire_Implementation(const FHitResult& HitResult)
{
	if (!IsLocallyControlled()) PlayFireAnimations();

	if (USGGameUserSettings::GetSGGameUserSettings()->bShowTracers)
	{
		SpawnTracer(HitResult);
	}

	PlayImpactEffects(HitResult);

	if (!IsLocallyControlled()) return;
	const AActor* HitActor = HitResult.GetActor();

	if (!IsValid(HitActor) || !HitActor->CanBeDamaged()) return;
	CosmeticPlayHitMarker();
}

void USGWeaponComponent::ServerReload_Implementation()
{
	if (!IsValid(Equipped) || Rounds == Equipped->MagazineCapacity || bIsReloading) return;

	bIsReloading = true;
	MultiReload();

	GetOwner()->GetWorldTimerManager().SetTimer(ReloadingHandle, this, &USGWeaponComponent::AuthFinishReload,
	                                            Equipped->ReloadTime, false);
}

void USGWeaponComponent::MultiReload_Implementation()
{
	check(IsValid(Equipped->WeaponReloadMontage))
	check(IsValid(Equipped->ThirdPersonReloadMontage))

	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	if (IsLocallyControlled())
	{
		UAnimInstance* WeaponAnimInstance = OwningCharacter->GetFirstPersonWeaponMesh()->GetAnimInstance();
		check(IsValid(WeaponAnimInstance))

		WeaponAnimInstance->Montage_Play(Equipped->WeaponReloadMontage,
		                                 Equipped->WeaponReloadMontage->GetPlayLength() / Equipped->ReloadTime);
	}
	else if (!IsLocallyControlled())
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

void USGWeaponComponent::AuthFinishReload()
{
	if (!HasAuthority()) return;

	bIsReloading = false;

	check(Equipped)
	Rounds = Equipped->MagazineCapacity;
}

void USGWeaponComponent::PlayFireAnimations() const
{
	check(IsValid(Equipped->FirstPersonFireMontage))
	check(IsValid(Equipped->WeaponFireMontage))

	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	if (IsLocallyControlled())
	{
		UAnimInstance* ArmsAnimInstance = OwningCharacter->GetArmsMesh()->GetAnimInstance();
		UAnimInstance* WeaponAnimInstance = OwningCharacter->GetFirstPersonWeaponMesh()->GetAnimInstance();
		check(IsValid(ArmsAnimInstance))
		check(IsValid(WeaponAnimInstance))

		ArmsAnimInstance->Montage_Play(Equipped->FirstPersonFireMontage);
		WeaponAnimInstance->Montage_Play(Equipped->WeaponFireMontage);
	}
	else if (!IsLocallyControlled())
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

	if (IsLocallyControlled())
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
	else if (!IsLocallyControlled())
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

void USGWeaponComponent::PlayImpactEffects(const FHitResult& HitResult) const
{
	check(IsValid(Equipped->BodyImpactCue))
	check(IsValid(Equipped->SurfaceImpactCue))

	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(OwningCharacter)

	const AActor* HitActor = HitResult.GetActor();
	if (!IsValid(HitActor)) return;

	UGameplayStatics::PlaySoundAtLocation(OwningCharacter,
											  HitActor->CanBeDamaged()
												  ? Equipped->BodyImpactCue
												  : Equipped->SurfaceImpactCue, HitResult.Location);

	if (!USGGameUserSettings::GetSGGameUserSettings()->bShowParticles)
	{
		return;
	}

	UParticleSystem* ImpactParticles = nullptr;

	if (HitActor->CanBeDamaged())
	{
		ImpactParticles = HitResult.BoneName == "head" ? Equipped->HeadShotParticles : Equipped->BodyShotParticles;
	}
	else ImpactParticles = Equipped->SurfaceImpactParticles;

	UGameplayStatics::SpawnEmitterAtLocation(OwningCharacter, ImpactParticles, HitResult.Location,
											 HitResult.Normal.Rotation());
}

void USGWeaponComponent::CosmeticPlayHitMarker()
{
	if (!IsLocallyControlled()) return;
	check(IsValid(HitMarker));

	UGameplayStatics::PlaySound2D(this, HitMarker);
}

void USGWeaponComponent::OnRep_Equipped() const
{
	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	OwningCharacter->GetFirstPersonWeaponMesh()->SetSkeletalMesh(!IsValid(Equipped) ? nullptr : Equipped->Mesh);
	OwningCharacter->GetThirdPersonWeaponMesh()->SetSkeletalMesh(!IsValid(Equipped) ? nullptr : Equipped->Mesh);
}
