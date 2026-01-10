#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SGTeamComponent.generated.h"

UENUM(BlueprintType)
enum class ETeam : uint8
{
	None,
	Red,
	Blue
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), DisplayName="Team Component", Blueprintable)
class SHOOTERGAME_API USGTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USGTeamComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void AuthSetTeam(const ETeam NewTeam);

	UFUNCTION(BlueprintPure)
	ETeam GetTeam() const { return Team; };

protected:
	UPROPERTY(EditDefaultsOnly)
	UMaterialInstance* RedTeamMaterial;

	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* BlueTeamMaterial;

private:
	UPROPERTY(ReplicatedUsing=OnRep_Team)
	ETeam Team;

protected:
	UFUNCTION()
	void OnRep_Team();
};
