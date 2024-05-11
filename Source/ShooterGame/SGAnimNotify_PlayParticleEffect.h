#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify_PlayParticleEffect.h"
#include "SGAnimNotify_PlayParticleEffect.generated.h"

UCLASS(DisplayName="Play Configurable Particle Effect")
class SHOOTERGAME_API USGAnimNotify_PlayParticleEffect : public UAnimNotify_PlayParticleEffect
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
