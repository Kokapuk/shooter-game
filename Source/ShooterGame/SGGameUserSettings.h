#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "SGGameUserSettings.generated.h"

UCLASS(Config = GameUserSettings)
class SHOOTERGAME_API USGGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, BlueprintReadOnly)
	float MouseSensitivity = .5f;

	UPROPERTY(Config, BlueprintReadOnly)
	float Volume = .5f;

	UFUNCTION(BlueprintPure)
	static USGGameUserSettings* GetSGGameUserSettings() { return Cast<USGGameUserSettings>(GetGameUserSettings()); };

	UFUNCTION(BlueprintCallable)
	void SetMouseSensitivity(const float NewSensitivity) { MouseSensitivity = NewSensitivity; }

	UFUNCTION(BlueprintCallable)
	void SetVolume(const float NewVolume) { Volume = NewVolume; }
};
