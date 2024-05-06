#include "SGPlayerController.h"

void ASGPlayerController::AddYawInput(float Value)
{
	Super::AddYawInput(Value * Sensitivity);
}

void ASGPlayerController::AddPitchInput(float Value)
{
	Super::AddPitchInput(Value * Sensitivity);
}

void ASGPlayerController::ClientResetControlRotation_Implementation(const FRotator& NewRotation)
{
	SetControlRotation(NewRotation);
}
