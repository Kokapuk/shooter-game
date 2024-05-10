#include "SGPlayerController.h"

#include "SGGameUserSettings.h"

void ASGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController()) CosmeticUpdateMouseSensitivity();
}

void ASGPlayerController::CosmeticUpdateMouseSensitivity()
{
	USGGameUserSettings* GameUserSettings = USGGameUserSettings::GetSGGameUserSettings();
	GameUserSettings->LoadSettings();
	
	MouseSensitivity = GameUserSettings->MouseSensitivity;
}

void ASGPlayerController::ClientResetControlRotation_Implementation(const FRotator& NewRotation)
{
	SetControlRotation(NewRotation);
}
