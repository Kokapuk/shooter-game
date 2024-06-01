#pragma once

#include "CoreMinimal.h"
#include "SGPlayerState.h"
#include "GameFramework/SpectatorPawn.h"
#include "SGSpectatorPawn.generated.h"

class ASGPlayerState;
class ASGCharacter;
class UCameraComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetSelect, ASGCharacter*, TargetCharacter);

UENUM(BlueprintType)
enum class ESpectatingMode : uint8
{
	Free,
	Observing
};

UCLASS()
class SHOOTERGAME_API ASGSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	ASGSpectatorPawn();

	UPROPERTY(BlueprintAssignable)
	FOnTargetSelect OnTargetSelect;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure)
	APlayerState* GetTargetPlayer() const { return TargetPlayer; }

	UFUNCTION(Client, Reliable)
	void ClientHideWidgets();

protected:
	UPROPERTY(EditDefaultsOnly)
	UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> BlindnessWidgetClass;

	ESpectatingMode SpectatingMode;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void UnsubscribeFromDeathEvent();
	void ShowWidgets();
	void HideWidgets() const;
	void ToggleSpectatingMode();
	TArray<ASGCharacter*> GetAliveCharacters() const;
	void SpectateNext();
	void CosmeticUpdateCameraRotation(const float DeltaTime) const;

private:
	float SavedMaxSpeed;

	UPROPERTY()
	ASGCharacter* TargetCharacter;

	UPROPERTY()
	ASGPlayerState* TargetPlayer;

	UPROPERTY()
	UUserWidget* HUDWidget;

	UPROPERTY()
	UUserWidget* BlindnessWidget;

	UFUNCTION()
	void HandleViewTargetDie() { ToggleSpectatingMode(); }
};
