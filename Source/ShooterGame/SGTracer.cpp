#include "SGTracer.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"

ASGTracer::ASGTracer()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
#if WITH_EDITORONLY_DATA
	RootComponent->bVisualizeComponent = true;
#endif
	
	ParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>("ParticleSystem");
	ParticleSystem->SetupAttachment(RootComponent);
	ParticleSystem->SetRelativeLocation(FVector(60.f, 0.f, 0.f));

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->MaxSpeed = 10000.f;
	ProjectileMovement->InitialSpeed = ProjectileMovement->MaxSpeed;
}

void ASGTracer::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	ParticleSystem->OnSystemFinished.AddUniqueDynamic(this, &ASGTracer::HandleSystemFinished);
}

void ASGTracer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float TraveledDistance = (GetActorLocation() - StartLocation).Size();
	if (TraveledDistance >= DistanceToTravel) Destroy();
}
