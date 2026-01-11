#pragma once

#include "SGLagCompensationComponent.generated.h"

class ASGCharacter;

USTRUCT(BlueprintType)
struct FSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float Timestamp;

	UPROPERTY(BlueprintReadOnly)
	FVector ActorLocation;

	UPROPERTY(BlueprintReadOnly)
	FPoseSnapshot Pose;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), DisplayName="Lag Compensation Component")
class SHOOTERGAME_API USGLagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USGLagCompensationComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure)
	int32 GetSnapshotBufferSize() const { return TargetTickrate * (SnapshotMaxValidDelay / 1000.f); }

	UFUNCTION(BlueprintPure)
	FSnapshot GetClosestSnapshot(const float TargetTimestamp);

protected:
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0, ClamMax=1000))
	uint16 SnapshotMaxValidDelay;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=32, ClamMax=256))
	uint16 TargetTickrate;

private:
	UPROPERTY()
	ASGCharacter* Character;

	TArray<FSnapshot> SnapshotBuffer;
	uint16 WriteIndex;
};
