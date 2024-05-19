#include "SGBlindnessComponent.h"

#include "SGGameState.h"
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
	check(GameState)

	GameState->OnMatchBegin.AddUniqueDynamic(this, &USGBlindnessComponent::HandleMatchBegin);
}

void USGBlindnessComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	BlindnessTimeline.TickTimeline(DeltaTime);
}

void USGBlindnessComponent::ClientBlind_Implementation(UCurveFloat* NewBlindnessCurve)
{
	BlindnessCurve = NewBlindnessCurve;

	FOnTimelineFloat OnBlindnessProgress;
	OnBlindnessProgress.BindUFunction(this, "HandleBlindnessProgress");
	BlindnessTimeline.AddInterpFloat(BlindnessCurve, OnBlindnessProgress);

	BlindnessTimeline.PlayFromStart();
}

void USGBlindnessComponent::HandleMatchBegin()
{
	const ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	if (!OwningCharacter->IsLocallyControlled()) return;

	UUserWidget* BlindnessWidget = CreateWidget(GetWorld(), BlindnessWidgetClass);
	BlindnessWidget->AddToViewport();
}

void USGBlindnessComponent::HandleBlindnessProgress()
{
	BlindnessScale = BlindnessCurve->GetFloatValue(BlindnessTimeline.GetPlaybackPosition());
}
