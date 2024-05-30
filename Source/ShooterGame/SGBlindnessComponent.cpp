#include "SGBlindnessComponent.h"

#include "SGGameState.h"
#include "SGPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

USGBlindnessComponent::USGBlindnessComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void USGBlindnessComponent::BeginPlay()
{
	Super::BeginPlay();

	ASGGameState* GameState = GetWorld()->GetGameState<ASGGameState>();
	check(IsValid(GameState))

	GameState->OnMatchBegin.AddUniqueDynamic(this, &USGBlindnessComponent::CosmeticHandleMatchBegin);
}

void USGBlindnessComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	BlindnessTimeline.TickTimeline(DeltaTime);
}

void USGBlindnessComponent::MultiBlind_Implementation(UCurveFloat* NewBlindnessCurve)
{
	const APawn* OwningPawn = GetOwner<APawn>();
	check(IsValid(OwningPawn))
	
	const APlayerState* PlayerState = GetWorld()->GetFirstPlayerController()->GetPlayerState<APlayerState>();
	check(IsValid(PlayerState))

	if (!OwningPawn->IsLocallyControlled() && !PlayerState->IsSpectator())
	{
		return;
	}
	
	BlindnessCurve = NewBlindnessCurve;

	FOnTimelineFloat OnBlindnessProgress;
	OnBlindnessProgress.BindUFunction(this, "HandleBlindnessProgress");
	BlindnessTimeline.AddInterpFloat(BlindnessCurve, OnBlindnessProgress);

	BlindnessTimeline.PlayFromStart();
}

void USGBlindnessComponent::CosmeticHandleMatchBegin()
{
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	check(IsValid(OwningPawn))

	if (!OwningPawn->IsLocallyControlled()) return;

	UUserWidget* BlindnessWidget = CreateWidget(GetWorld(), BlindnessWidgetClass);
	BlindnessWidget->AddToViewport();

	ASGPlayerState* PlayerState = OwningPawn->GetPlayerState<ASGPlayerState>();
	check(IsValid(PlayerState))

	PlayerState->OnDie.AddUniqueDynamic(this, &USGBlindnessComponent::CosmeticHandleOwningPlayerDie);
}

void USGBlindnessComponent::CosmeticHandleOwningPlayerDie()
{
	BlindnessTimeline.Stop();
	BlindnessScale = 0.f;
}
