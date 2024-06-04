#pragma once

#include "CoreMinimal.h"
#include "ShooterGame/Enums.h"
#include "ShooterGame/SGCharacter.h"
#include "SGVersusCharacter.generated.h"

UCLASS()
class SHOOTERGAME_API ASGVersusCharacter : public ASGCharacter
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual bool ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const override;

	UFUNCTION(BlueprintPure)
	ETeam GetTeam() const;

protected:
	UPROPERTY(EditDefaultsOnly)
	UMaterialInstance* RedTeamMaterial;
	
	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* BlueTeamMaterial;

	UPROPERTY(ReplicatedUsing=OnRep_Team)
	ETeam Team;

	UFUNCTION()
	void OnRep_Team();
};
