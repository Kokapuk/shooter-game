#pragma once

#include "CoreMinimal.h"
#include "SGCharacter.h"
#include "GameFramework/PlayerState.h"
#include "SGPlayerState.generated.h"

class ASGCharacter;

UENUM(BlueprintType)
enum class ETeam : uint8
{
	None,
	Red,
	Blue
};

UCLASS()
class SHOOTERGAME_API ASGPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="AssingTeam")
	void AuthAssignTeam(const ETeam NewTeam);

	UFUNCTION(BlueprintPure)
	ETeam GetTeam() const { return Team; }

	UFUNCTION(BlueprintPure)
	ASGCharacter* GetControlledCharacter() const { return GetPawn<ASGCharacter>(); }

	UFUNCTION(BlueprintPure)
	UMaterialInterface* GetMaterialByTeam() const;

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UMaterialInterface* NoTeamMaterial;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UMaterialInterface* RedTeamMaterial;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UMaterialInterface* BlueTeamMaterial;

	UPROPERTY(Replicated, BlueprintReadOnly, VisibleInstanceOnly)
	ETeam Team;
};
