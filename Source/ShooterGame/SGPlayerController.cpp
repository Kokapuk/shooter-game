#include "SGPlayerController.h"

#include "SGGameUserSettings.h"
#include <Kismet/GameplayStatics.h>

void ASGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController()) CosmeticLoadSettings();
}

void ASGPlayerController::ClientResetControlRotation_Implementation(const FRotator& NewRotation)
{
	SetControlRotation(NewRotation);
}

void ASGPlayerController::SetMouseSensitivity(float Value) {
	MouseSensitivity = Value;
}