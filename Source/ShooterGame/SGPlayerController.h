#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SGPlayerController.generated.h"

UCLASS()
class SHOOTERGAME_API ASGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void AddYawInput(float Value) override { Super::AddYawInput(Value * MouseSensitivity); }
	virtual void AddPitchInput(float Value) override { Super::AddPitchInput(Value * MouseSensitivity); }

	UFUNCTION(Client, Unreliable)
	void ClientResetControlRotation(const FRotator& NewRotation);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, DisplayName="Update Mouse Sensitivity")
	void CosmeticUpdateMouseSensitivity();

private:
	float MouseSensitivity;
};
