#include "SGAnimNotify_PlayParticleEffect.h"

#include "SGGameUserSettings.h"

void USGAnimNotify_PlayParticleEffect::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
#if !WITH_EDITOR
	if (!USGGameUserSettings::GetSGGameUserSettings()->bShowParticles) return;
#endif

	Super::Notify(MeshComp, Animation);
}
