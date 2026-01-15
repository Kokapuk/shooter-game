#include "SGLagCompensationComponent.h"

#include "SGCharacter.h"
#include "SGPlayerState.h"

USGLagCompensationComponent::USGLagCompensationComponent()
{
	SnapshotMaxValidDelay = 200;
	TargetTickrate = 128;
}

void USGLagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	SnapshotBuffer.SetNum(GetSnapshotBufferSize());
	Character = GetOwner<ASGCharacter>();
}

FSnapshot USGLagCompensationComponent::GetClosestSnapshot(const float TargetTimestamp) const
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

	return SnapshotBuffer[ClosestSnapshotIndex];
}

FSnapshot USGLagCompensationComponent::GetSnapshotForPlayer(const ASGPlayerState* Player) const
{
	return GetClosestSnapshot(GetWorld()->GetTimeSeconds() - Player->GetRoundTripTime());
}


void USGLagCompensationComponent::CaptureSnapshot()
{
	if (WriteIndex >= SnapshotBuffer.Num()) WriteIndex = 0;

	const FSnapshot Snapshot = FSnapshot(GetWorld()->GetTimeSeconds(), Character->GetActorLocation());

	SnapshotBuffer[WriteIndex] = Snapshot;
	WriteIndex++;
}
