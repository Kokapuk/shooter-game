#include "SGLagCompensationComponent.h"

#include "SGCharacter.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"

USGLagCompensationComponent::USGLagCompensationComponent()
{
	// PrimaryComponentTick.bCanEverTick = true;
	SnapshotMaxValidDelay = 300;
	TargetTickrate = 128;
}

void USGLagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	SnapshotBuffer.SetNum(GetSnapshotBufferSize());
	Character = GetOwner<ASGCharacter>();
}

// void USGLagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
//                                                 FActorComponentTickFunction* ThisTickFunction)
// {
// 	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
// 	if (!Character->HasAuthority()) return;
// 	CaptureSnapshot();
// }

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

TArray<FSkeletonHitbox> USGLagCompensationComponent::CaptureSkeletonHitboxes()
{
	const USkeletalMeshComponent* Mesh = Character->GetMesh();
	check(IsValid(Mesh));

	UPhysicsAsset* PhysicsAsset = Mesh->GetPhysicsAsset();
	check(IsValid(PhysicsAsset));

	TArray<FSkeletonHitbox> Hitboxes;

	for (const USkeletalBodySetup* BodySetup : PhysicsAsset->SkeletalBodySetups)
	{
		FName BoneName = BodySetup->BoneName;
		int32 BoneIndex = Mesh->GetBoneIndex(BoneName);

		FTransform BoneWorldTransform = Mesh->GetBoneTransform(BoneIndex);

		for (const FKBoxElem& Box : BodySetup->AggGeom.BoxElems)
		{
			FTransform RelativeTransform = Box.GetTransform();
			FTransform HitboxWorldTransform = RelativeTransform * BoneWorldTransform;

			FSkeletonHitbox Hitbox;
			Hitbox.Name = Box.GetName();
			Hitbox.Transform = HitboxWorldTransform;
			Hitbox.Type = EHitboxType::Box;
			Hitbox.BoxExtents = FVector(Box.X, Box.Y, Box.Z);

			Hitboxes.Push(Hitbox);
		}

		for (const FKSphylElem& Capsule : BodySetup->AggGeom.SphylElems)
		{
			FTransform RelativeTransform = Capsule.GetTransform();
			FTransform HitboxWorldTransform = RelativeTransform * BoneWorldTransform;

			float TotalHeight = Capsule.Length + (2.0f * Capsule.Radius);
			float HalfHeight = TotalHeight / 2.0f;

			FSkeletonHitbox Hitbox;
			Hitbox.Name = Capsule.GetName();
			Hitbox.Transform = HitboxWorldTransform;
			Hitbox.Type = EHitboxType::Capsule;
			Hitbox.Radius = Capsule.Radius;
			Hitbox.HalfHeight = HalfHeight;

			Hitboxes.Push(Hitbox);
		}
	}

	return Hitboxes;
}

void USGLagCompensationComponent::CaptureSnapshot()
{
	if (WriteIndex >= SnapshotBuffer.Num()) WriteIndex = 0;

	FSnapshot Snapshot = FSnapshot(GetWorld()->GetTimeSeconds(), Character->GetActorLocation());
	Snapshot.Hitboxes = CaptureSkeletonHitboxes();

	SnapshotBuffer[WriteIndex] = Snapshot;
	WriteIndex++;
}
