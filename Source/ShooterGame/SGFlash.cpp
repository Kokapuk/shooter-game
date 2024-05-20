#include "SGFlash.h"

#include "EngineUtils.h"
#include "SGBlindnessComponent.h"
#include "SGCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/TimelineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"

ASGFlash::ASGFlash()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->SetCastShadow(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetGenerateOverlapEvents(false);

	Light = CreateDefaultSubobject<UPointLightComponent>("Light");
	Light->SetupAttachment(RootComponent);

	MaxScale = .35f;
	MaxLightIntensity = 15000.f;
	MaxLocationOffset = 275.f;
}

void ASGFlash::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();

	check(IsValid(ChargeCurve))

	FOnTimelineFloat OnChargeProgress;
	OnChargeProgress.BindUFunction(this, "HandleChargeProgress");
	ChargeTimeline.AddInterpFloat(ChargeCurve, OnChargeProgress);

	if (HasAuthority())
	{
		FOnTimelineEventStatic OnChargeFinished;
		OnChargeFinished.BindUFunction(this, "AuthExplode");
		ChargeTimeline.SetTimelineFinishedFunc(OnChargeFinished);
	}

	ChargeTimeline.PlayFromStart();

	check(IsValid(ChargingCue))
	UGameplayStatics::SpawnSoundAttached(ChargingCue, RootComponent, NAME_None, FVector::ZeroVector,
	                                     FRotator::ZeroRotator,
	                                     EAttachLocation::SnapToTarget, false, 1,
	                                     ChargingCue->GetDuration() / ChargeTimeline.GetTimelineLength());
}

void ASGFlash::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ChargeTimeline.TickTimeline(DeltaSeconds);
}

void ASGFlash::HandleChargeProgress()
{
	const float Progress = ChargeCurve->GetFloatValue(ChargeTimeline.GetPlaybackPosition());

	Mesh->SetRelativeScale3D(FVector(MaxScale * Progress));
	Light->SetIntensity(MaxLightIntensity * Progress);
	SetActorLocation(StartLocation + GetActorUpVector() * (MaxLocationOffset * Progress));
}

void ASGFlash::AuthExplode()
{
	if (!HasAuthority()) return;

	bool bBlinded = false;

	for (TActorIterator<ASGCharacter> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		const UCameraComponent* Camera = Iterator->GetCamera();
		check(IsValid(Camera))

		FHitResult HitResult;
		const bool bHit = UKismetSystemLibrary::LineTraceSingle(this, GetActorLocation(),
		                                                        Camera->GetComponentLocation(),
		                                                        UEngineTypes::ConvertToTraceType(ECC_Visibility), false,
		                                                        {*Iterator}, EDrawDebugTrace::ForDuration, HitResult,
		                                                        true, FLinearColor::Yellow, FLinearColor::Blue, 2.f);

		if (bHit) continue;

		const FRotator BaseAimRotation = Iterator->GetBaseAimRotation();
		const float ClampedAimPitch = UKismetMathLibrary::ClampAngle(BaseAimRotation.Pitch, -90.f, 90.f);
		const float ClampedAimYaw = UKismetMathLibrary::ClampAngle(BaseAimRotation.Yaw, -179.9f, 179.9f);
		const FRotator ClampedAimRotation = FRotator(ClampedAimPitch, ClampedAimYaw, 0.f);
		const FRotator CameraLookAtFlashRotation = UKismetMathLibrary::FindLookAtRotation(
			Camera->GetComponentLocation(), GetActorLocation());
		const FRotator Diff = ClampedAimRotation - CameraLookAtFlashRotation;
		const float HalfFieldOfView = Camera->FieldOfView / 2.f;
		constexpr float TargetAspectRatio = 16.f / 9.f;

		if (FMath::Abs(UKismetMathLibrary::ClampAngle(Diff.Pitch * TargetAspectRatio, -90.f, 90.f)) > HalfFieldOfView ||
			FMath::Abs(FMath::ClampAngle(Diff.Yaw, -179.9f, 179.9f)) > HalfFieldOfView)
		{
			continue;
		}


		check(IsValid(BlindnessCurve))
		Iterator->GetBlindnessComponent()->ClientBlind(BlindnessCurve);
		bBlinded = true;
	}

	MultiExplode(bBlinded);
	Destroy();
}

void ASGFlash::MultiExplode_Implementation(const bool bBlinded)
{
	if (bBlinded)
	{
		check(IsValid(BlindedCue))
		UGameplayStatics::PlaySoundAtLocation(this, BlindedCue, GetActorLocation(), FRotator::ZeroRotator);
	}
	else
	{
		check(IsValid(ExplosionCue))
		UGameplayStatics::PlaySoundAtLocation(this, ExplosionCue, GetActorLocation(), FRotator::ZeroRotator);
	}
}
