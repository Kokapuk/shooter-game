#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "SGBlindnessComponent.generated.h"

struct FTimeline;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SHOOTERGAME_API USGBlindnessComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USGBlindnessComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, DisplayName="Blind")
	void MultiBlind(UCurveFloat* NewBlindnessCurve);

	UFUNCTION(BlueprintPure)
	float GetBlindnessScale() const { return BlindnessScale; }

	UFUNCTION(NetMulticast, Reliable, DisplayName="Reset")
	void MultiReset();

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> BlindnessWidgetClass;

private:

	FTimeline BlindnessTimeline;

	UPROPERTY()
	UCurveFloat* BlindnessCurve;

	float BlindnessScale;

	UFUNCTION(BlueprintCosmetic, DisplayName="HandleOwningPlayerDie")
	void HandleMatchBegin();

	UFUNCTION(BlueprintCosmetic, DisplayName="HandleOwningPlayerDie")
	void HandleOwningCharacterDie();

	UFUNCTION()
	void HandleBlindnessProgress(const float Scale) { BlindnessScale = Scale; }
};
