#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "SGGameUserSettings.generated.h"

UCLASS(Config = GameUserSettings)
class SHOOTERGAME_API USGGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	USGGameUserSettings();

	UPROPERTY(Config, BlueprintReadOnly)
	float MouseSensitivity;

	UPROPERTY(Config, BlueprintReadOnly)
	float Volume;

	UPROPERTY(Config, BlueprintReadOnly)
	uint8 bShowParticles : 1;

	UPROPERTY(Config, BlueprintReadOnly)
	uint8 bShowTracers : 1;

	UFUNCTION(BlueprintPure)
	static USGGameUserSettings* GetSGGameUserSettings() { return Cast<USGGameUserSettings>(GetGameUserSettings()); };

	UFUNCTION(BlueprintCallable)
	void SetMouseSensitivity(const float NewSensitivity) { MouseSensitivity = NewSensitivity; }

	UFUNCTION(BlueprintCallable)
	void SetVolume(const float NewVolume) { Volume = NewVolume; }

	UFUNCTION(BlueprintCallable)
	void SetShowParticles(const bool bNewShowParticles) { bShowParticles = bNewShowParticles; }

	UFUNCTION(BlueprintCallable)
	void SetShowTracers(const bool bNewShowTracers) { bShowTracers = bNewShowTracers; }
};
