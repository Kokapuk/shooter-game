#include "SGWeaponDataAsset.h"

USGWeaponDataAsset::USGWeaponDataAsset()
{
	TimeBetweenShots = 0.f;
	MaxShootingError = 0.f;;
	ReloadTime = 0.f;
	MagazineCapacity = 1;
	HeadShotDamage = 0.f;
	BodyShotDamage = 0.f;
	Mesh = nullptr;
	WeaponFireMontage = nullptr;
	FirstPersonFireMontage = nullptr;
	WeaponReloadMontage = nullptr;
	ThirdPersonReloadMontage = nullptr;
  FirstPersonReloadMontage = nullptr;
	TracerClass = nullptr;
	SurfaceImpactCue = nullptr;
	SurfaceImpactParticles = nullptr;
	BodyImpactCue = nullptr;
	BodyImpactParticles = nullptr;
}
