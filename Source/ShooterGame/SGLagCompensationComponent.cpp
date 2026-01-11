#include "SGLagCompensationComponent.h"

#include "SGCharacter.h"

USGLagCompensationComponent::USGLagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SnapshotMaxValidDelay = 300;
	TargetTickrate = 128;
}

void USGLagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	SnapshotBuffer.SetNum(GetSnapshotBufferSize());
	Character = GetOwner<ASGCharacter>();
}

void USGLagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Character->HasAuthority()) return;

	if (WriteIndex >= SnapshotBuffer.Num()) WriteIndex = 0;

	FSnapshot Snapshot = FSnapshot(GetWorld()->GetTimeSeconds(), Character->GetActorLocation());
	Character->GetMesh()->SnapshotPose(Snapshot.Pose);
	SnapshotBuffer[WriteIndex] = Snapshot;
	WriteIndex++;
}

FSnapshot USGLagCompensationComponent::GetClosestSnapshot(const float TargetTimestamp)
{
	int32 ClosestSnapshotIndex = 0;
	float SmallestTimestampDiff = SnapshotMaxValidDelay + 1.f;

	for (int32 i = 0; i < SnapshotBuffer.Num(); ++i)
	{
		const FSnapshot Snapshot = SnapshotBuffer[i];
		const float TimestampDiff = FMath::Abs(TargetTimestamp - Snapshot.Timestamp);

		if (TimestampDiff < SmallestTimestampDiff)
		{
			SmallestTimestampDiff = TimestampDiff;
			ClosestSnapshotIndex = i;
		}
	}

	if (SmallestTimestampDiff > SnapshotMaxValidDelay) return FSnapshot();
	return SnapshotBuffer[ClosestSnapshotIndex];
}
