#include "SGWeaponComponent.h"

#include "SGCharacter.h"
#include "SGGameUserSettings.h"
#include "SGTracer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DamageEvents.h"

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
	DOREPLIFETIME(USGWeaponComponent, Rounds);
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
	const ASGCharacter* OwningCharacter = GetOwner<ASGCharacter>();
	check(IsValid(OwningCharacter))

	return OwningCharacter->IsLocallyControlled();
}

bool USGWeaponComponent::IsOwnerLocalViewTarget() const
{
	const APlayerController* LocalPlayerController = GetWorld()->GetFirstPlayerController();
	check(IsValid(LocalPlayerController))

	return LocalPlayerController->GetViewTarget() == GetOwner();
}

void USGWeaponComponent::ServerEquip_Implementation(USGWeaponDataAsset* Weapon)
{
	GetOwner()->GetWorldTimerManager().ClearTimer(ReloadingHandle);
	bIsReloading = false;
	TimeToFire = 0.f;

	Equipped = Weapon;
	OnRep_Equipped();
	AuthResetRounds();
}

bool USGWeaponComponent::CanFire() const
{
	return (HasAuthority() || IsLocallyControlled()) && IsValid(Equipped) && Rounds > 0 && TimeToFire == 0.f && !
		bIsReloading;
}

void USGWeaponComponent::AuthResetRounds()
{
	if (!HasAuthority())
	{
		return;
	}
	
	Rounds = IsValid(Equipped) ? Equipped->MagazineCapacity : 0;
}

bool USGWeaponComponent::CanReload() const
{
	return IsValid(Equipped) && Rounds < Equipped->MagazineCapacity && !bIsReloading;
}

void USGWeaponComponent::CosmeticFire()
{
	if (!IsLocallyControlled())
	{
		return;
	}
	
	if (!Rounds)
	{
		return ServerReload();
	}
	
	if (!CanFire())
	{
		return;
	}

	if (!HasAuthority()) TimeToFire = Equipped->TimeBetweenShots;
	PlayFireAnimations();

	const ASGCharacter* OwningCharacter = GetOwner<ASGCharacter>();
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

void USGWeaponComponent::CosmeticReload()
{
	if (!IsLocallyControlled() || !CanReload())
	{
		return;
	}

	ServerReload();
}

void USGWeaponComponent::ServerFire_Implementation(const FHitResult& HitResult)
{
	if (!CanFire()) return;

	TimeToFire = Equipped->TimeBetweenShots;
	Rounds--;

	if (!Rounds)
	{
		const float FireAnimationLength = GetEquipped()->WeaponFireMontage->GetPlayLength();

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, this, &USGWeaponComponent::ServerReload_Implementation,
		                                       FireAnimationLength);
	}

	MultiFire(HitResult);

	if (!HitResult.bBlockingHit) return;

	AActor* HitActor = HitResult.GetActor();
	if (!IsValid(HitActor) || !HitActor->CanBeDamaged()) return;

	ASGCharacter* HitCharacter = Cast<ASGCharacter>(HitActor);

	if (IsValid(HitCharacter))
	{
		HitCharacter->MultiPlayHitReactMontage(HitResult.BoneName);
	}

	ASGCharacter* OwningCharacter = GetOwner<ASGCharacter>();
	check(IsValid(OwningCharacter))

	HitActor->TakeDamage(HitResult.BoneName == "head" ? Equipped->HeadShotDamage : Equipped->BodyShotDamage,
	                     FDamageEvent(), OwningCharacter->GetController(), OwningCharacter);
}

FVector USGWeaponComponent::GetFireDirection() const
{
	const ASGCharacter* OwningCharacter = GetOwner<ASGCharacter>();
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
	TimeToFire = 0.f;
	AuthResetRounds();
}

void USGWeaponComponent::MultiFire_Implementation(const FHitResult& HitResult)
{
	if (!IsLocallyControlled())
	{
		PlayFireAnimations();
	}

	if (USGGameUserSettings::GetSGGameUserSettings()->bShowTracers)
	{
		SpawnTracer(HitResult);
	}

	PlayImpactEffects(HitResult);

	const AActor* HitActor = HitResult.GetActor();

	if (IsValid(HitActor) && HitActor->CanBeDamaged() && IsOwnerLocalViewTarget())
	{
		PlayHitMarker();
	}

	OnFire.Broadcast();
}

void USGWeaponComponent::ServerReload_Implementation()
{
	if (!CanReload())
	{
		return;
	}

	bIsReloading = true;
	MultiReload();

	GetOwner()->GetWorldTimerManager().SetTimer(ReloadingHandle, this, &USGWeaponComponent::AuthFinishReload,
	                                            Equipped->ReloadTime, false);
}

void USGWeaponComponent::MultiReload_Implementation()
{
	check(IsValid(Equipped->WeaponReloadMontage))
	check(IsValid(Equipped->ThirdPersonReloadMontage))

	const ASGCharacter* OwningCharacter = GetOwner<ASGCharacter>();
	check(IsValid(OwningCharacter))

	if (IsOwnerLocalViewTarget())
	{
		UAnimInstance* WeaponAnimInstance = OwningCharacter->GetFirstPersonWeaponMesh()->GetAnimInstance();
		UAnimInstance* ArmsAnimInstance = OwningCharacter->GetArmsMesh()->GetAnimInstance();
		check(IsValid(WeaponAnimInstance))
		check(IsValid(ArmsAnimInstance))

		WeaponAnimInstance->Montage_Play(Equipped->WeaponReloadMontage,
		                                 Equipped->WeaponReloadMontage->GetPlayLength() / Equipped->ReloadTime);
		ArmsAnimInstance->Montage_Play(Equipped->FirstPersonReloadMontage,
		                                 Equipped->FirstPersonReloadMontage->GetPlayLength() / Equipped->ReloadTime);
	}
	else
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
	AuthResetRounds();
}

void USGWeaponComponent::PlayFireAnimations() const
{
	check(IsValid(Equipped->FirstPersonFireMontage))
	check(IsValid(Equipped->WeaponFireMontage))

	const ASGCharacter* OwningCharacter = GetOwner<ASGCharacter>();
	check(IsValid(OwningCharacter))

	if (IsOwnerLocalViewTarget())
	{
		UAnimInstance* ArmsAnimInstance = OwningCharacter->GetArmsMesh()->GetAnimInstance();
		UAnimInstance* WeaponAnimInstance = OwningCharacter->GetFirstPersonWeaponMesh()->GetAnimInstance();
		check(IsValid(ArmsAnimInstance))
		check(IsValid(WeaponAnimInstance))

		ArmsAnimInstance->Montage_Play(Equipped->FirstPersonFireMontage);
		WeaponAnimInstance->Montage_Play(Equipped->WeaponFireMontage);
	}
	else
	{
		UAnimInstance* WeaponAnimInstance = OwningCharacter->GetThirdPersonWeaponMesh()->GetAnimInstance();
		check(IsValid(WeaponAnimInstance))

		WeaponAnimInstance->Montage_Play(Equipped->WeaponFireMontage);
	}
}

void USGWeaponComponent::SpawnTracer(const FHitResult& HitResult) const
{
	const ASGCharacter* OwningCharacter = GetOwner<ASGCharacter>();
	check(IsValid(OwningCharacter))

	const FVector Start = HitResult.TraceStart;
	const FVector End = HitResult.bBlockingHit ? HitResult.Location : HitResult.TraceEnd;
	USkeletalMeshComponent* WeaponMesh;

	if (IsOwnerLocalViewTarget())
	{
		WeaponMesh = OwningCharacter->GetFirstPersonWeaponMesh();
	}
	else
	{
		WeaponMesh = OwningCharacter->GetThirdPersonWeaponMesh();
	}

	check(IsValid(WeaponMesh))

	const FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(WeaponMesh->GetComponentLocation(), End);
	const FTransform Transform = FTransform(Rotation, WeaponMesh->GetSocketLocation("MuzzleFlash"));

	ASGTracer* Tracer = GetWorld()->SpawnActorDeferred<ASGTracer>(Equipped->TracerClass, Transform);
	Tracer->SetDistanceToTravel((End - Start).Size());
	Tracer->FinishSpawning(Transform);
}

void USGWeaponComponent::PlayImpactEffects(const FHitResult& HitResult) const
{
	check(IsValid(Equipped->BodyImpactCue))
	check(IsValid(Equipped->SurfaceImpactCue))

	const ASGCharacter* OwningCharacter = GetOwner<ASGCharacter>();
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
		ImpactParticles = Equipped->BodyImpactParticles;
	}
	else
	{
		ImpactParticles = Equipped->SurfaceImpactParticles;
	}

	UGameplayStatics::SpawnEmitterAtLocation(OwningCharacter, ImpactParticles, HitResult.Location,
	                                         HitResult.Normal.Rotation());
}

void USGWeaponComponent::PlayHitMarker() const
{
	check(IsValid(HitMarker));

	UGameplayStatics::PlaySound2D(this, HitMarker);
}

void USGWeaponComponent::OnRep_Equipped() const
{
	const ASGCharacter* OwningCharacter = GetOwner<ASGCharacter>();
	check(IsValid(OwningCharacter))

	OwningCharacter->GetFirstPersonWeaponMesh()->SetSkeletalMesh(!IsValid(Equipped) ? nullptr : Equipped->Mesh);
	OwningCharacter->GetThirdPersonWeaponMesh()->SetSkeletalMesh(!IsValid(Equipped) ? nullptr : Equipped->Mesh);
}
