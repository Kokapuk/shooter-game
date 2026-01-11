#include "SGPlayerController.h"
#include "SGCheatManager.h"

ASGPlayerController::ASGPlayerController()
{
	CheatClass = USGCheatManager::StaticClass();
}

void ASGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController()) CosmeticLoadSettings();
}

void ASGPlayerController::ClientResetControlRotation_Implementation(const FRotator& NewRotation)
{
	SetControlRotation(NewRotation);
}

void ASGPlayerController::SetMouseSensitivity(float Value)
{
	MouseSensitivity = Value;
}
