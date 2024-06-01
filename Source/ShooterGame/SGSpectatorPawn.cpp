#include "SGSpectatorPawn.h"

#include "EngineUtils.h"
#include "SGCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpectatorPawnMovement.h"

ASGSpectatorPawn::ASGSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(RootComponent);
	Camera->SetFieldOfView(103.f);
	Camera->bUsePawnControlRotation = true;
}

void ASGSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();

	const USpectatorPawnMovement* DetailedMovementComponent = Cast<USpectatorPawnMovement>(GetMovementComponent());
	check(IsValid(DetailedMovementComponent))

	SavedMaxSpeed = DetailedMovementComponent->MaxSpeed;
}

void ASGSpectatorPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CosmeticUpdateCameraRotation(DeltaSeconds);
}

void ASGSpectatorPawn::ClientHideWidgets_Implementation()
{
	HideWidgets();
}

void ASGSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("ToggleSpectatingMode", IE_Pressed, this, &ASGSpectatorPawn::ToggleSpectatingMode);
	PlayerInputComponent->BindAction("SpectateNext", IE_Pressed, this, &ASGSpectatorPawn::SpectateNext);
}

void ASGSpectatorPawn::UnsubscribeFromDeathEvent()
{
	check(IsValid(TargetPlayer))

	TargetPlayer->OnDie.RemoveDynamic(this, &ASGSpectatorPawn::HandleViewTargetDie);
	TargetPlayer = nullptr;
}

void ASGSpectatorPawn::ShowWidgets()
{
	// HUD
	if (!IsValid(HUDWidget))
	{
		check(IsValid(HUDWidgetClass))

		HUDWidget = CreateWidget(GetWorld(), HUDWidgetClass);
	}

	if (!HUDWidget->IsInViewport())
	{
		HUDWidget->AddToViewport();
	}

	// Blindness
	if (!IsValid(BlindnessWidget))
	{
		check(IsValid(BlindnessWidgetClass))

		BlindnessWidget = CreateWidget(GetWorld(), BlindnessWidgetClass);
	}

	if (!BlindnessWidget->IsInViewport())
	{
		BlindnessWidget->SetRenderOpacity(.65f);
		BlindnessWidget->AddToViewport();
	}
}

void ASGSpectatorPawn::HideWidgets() const
{
	if (IsValid(HUDWidget) && HUDWidget->IsInViewport())
	{
		HUDWidget->RemoveFromParent();
	}

	if (IsValid(BlindnessWidget) && BlindnessWidget->IsInViewport())
	{
		BlindnessWidget->RemoveFromParent();
	}
}

void ASGSpectatorPawn::ToggleSpectatingMode()
{
	UFloatingPawnMovement* DetailedMovementComponent = Cast<UFloatingPawnMovement>(GetMovementComponent());
	check(IsValid(DetailedMovementComponent))

	APlayerController* PlayerController = GetController<APlayerController>();
	check(IsValid(PlayerController))

	if (SpectatingMode == ESpectatingMode::Observing)
	{
		check(IsValid(TargetCharacter))

		SpectatingMode = ESpectatingMode::Free;
		
		HideWidgets();
		UnsubscribeFromDeathEvent();
		SetActorLocation(TargetCharacter->GetCamera()->GetComponentLocation());
		GetController()->SetControlRotation(TargetCharacter->GetBaseAimRotation());

		TargetCharacter = nullptr;
		DetailedMovementComponent->MaxSpeed = SavedMaxSpeed;
		PlayerController->SetViewTarget(this);
	}
	else if (SpectatingMode == ESpectatingMode::Free && GetAliveCharacters().Num() > 0)
	{
		SpectatingMode = ESpectatingMode::Observing;
		
		DetailedMovementComponent->MaxSpeed = 0.f;
		SpectateNext();
	}
}

TArray<ASGCharacter*> ASGSpectatorPawn::GetAliveCharacters() const
{
	TArray<ASGCharacter*> AliveCharacters;

	for (TActorIterator<ASGCharacter> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
	{
		if (ActorIterator->IsDead()) continue;

		AliveCharacters.Add(*ActorIterator);
	}

	return AliveCharacters;
}

void ASGSpectatorPawn::SpectateNext()
{
	if (SpectatingMode == ESpectatingMode::Free)
	{
		return;
	}

	const TArray<ASGCharacter*> AliveCharacters = GetAliveCharacters();

	if (AliveCharacters.Num() == 0)
	{
		return;
	}

	int32 NextTargetIndex = 0;

	if (IsValid(TargetCharacter))
	{
		NextTargetIndex = AliveCharacters.Find(TargetCharacter) + 1;

		if (NextTargetIndex >= AliveCharacters.Num())
		{
			NextTargetIndex = 0;
		}

		UnsubscribeFromDeathEvent();
	}

	TargetCharacter = GetAliveCharacters()[NextTargetIndex];
	check(IsValid(TargetCharacter))

	UCameraComponent* TargetCamera = TargetCharacter->GetCamera();
	check(IsValid(TargetCamera))

	TargetCamera->SetUsingAbsoluteRotation(true);

	TargetPlayer = TargetCharacter->GetPlayerState<ASGPlayerState>();
	check(IsValid(TargetPlayer))

	TargetPlayer->OnDie.AddUniqueDynamic(this, &ASGSpectatorPawn::HandleViewTargetDie);

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	check(IsValid(PlayerController))

	PlayerController->SetViewTarget(TargetCharacter);

	ShowWidgets();
	OnTargetSelect.Broadcast(TargetCharacter);
}

void ASGSpectatorPawn::CosmeticUpdateCameraRotation(const float DeltaTime) const
{
	if (!IsValid(TargetCharacter)) return;

	UCameraComponent* TargetCamera = TargetCharacter->GetCamera();

	const FRotator NewRotation = FMath::RInterpTo(TargetCamera->GetComponentRotation(),
	                                              TargetCharacter->GetBaseAimRotation(), DeltaTime, 35.f);
	TargetCamera->SetWorldRotation(NewRotation);
}
