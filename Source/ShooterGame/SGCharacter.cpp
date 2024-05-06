#include "SGCharacter.h"

#include "SGGameMode.h"
#include "SGGameState.h"
#include "SGPlayerState.h"
#include "SGWeaponComponent.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

ASGCharacter::ASGCharacter()
{
	MaxHealth = 100.f;
	BaseEyeHeight = 166.f;
	CrouchedEyeHeight = 115.f;
	TargetCameraHeight = BaseEyeHeight;

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	Capsule->SetCapsuleHalfHeight(96.f);
	Capsule->SetCapsuleRadius(55.f);

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->NavAgentProps.bCanCrouch = true;
	MovementComponent->MaxWalkSpeed = 540.f;
	MovementComponent->MaxWalkSpeedCrouched = 240.f;
	MovementComponent->AirControl = 1.f;
	MovementComponent->CrouchedHalfHeight = 60.f;

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	CharacterMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	CharacterMesh->SetRelativeLocation(FVector(0.f, 0.f, -Capsule->GetScaledCapsuleHalfHeight()));

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->AttachToComponent(CharacterMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true));
	Camera->SetFieldOfView(103);
	Camera->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	Camera->SetRelativeScale3D(FVector(.6f));
	Camera->bUsePawnControlRotation = true;

	ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ArmsMesh");
	ArmsMesh->SetOnlyOwnerSee(true);
	ArmsMesh->SetupAttachment(Camera);
	ArmsMesh->bCastDynamicShadow = false;
	ArmsMesh->CastShadow = false;

	FirstPersonWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("FirstPersonGunMesh");
	FirstPersonWeaponMesh->SetOnlyOwnerSee(true);
	FirstPersonWeaponMesh->bCastDynamicShadow = false;
	FirstPersonWeaponMesh->CastShadow = false;
	FirstPersonWeaponMesh->AttachToComponent(ArmsMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
	                                         "GripPoint");

	ThirdPersonWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ThirdPersonGunMesh");
	ThirdPersonWeaponMesh->SetOwnerNoSee(true);
	ThirdPersonWeaponMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
	                                         "GripPoint");

	Weapon = CreateDefaultSubobject<USGWeaponComponent>("Weapon");
}

void ASGCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Camera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight));
}

void ASGCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGCharacter, Health);
	DOREPLIFETIME(ASGCharacter, bIsDead);
}

void ASGCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	const UWorld* World = GetWorld();
	if (!IsValid(World)) return;

	ASGGameState* GameState = World->GetGameState<ASGGameState>();
	if (!IsValid(GameState)) return;

	GameState->OnMatchBegin.AddUniqueDynamic(this, &ASGCharacter::HandleMatchBegin);
}

void ASGCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const FVector NewCameraLocation = UKismetMathLibrary::VInterpTo(Camera->GetRelativeLocation(),
	                                                                FVector(0.f, 0.f, TargetCameraHeight), DeltaSeconds,
	                                                                10.f);
	Camera->SetRelativeLocation(NewCameraLocation);
}

bool ASGCharacter::ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                    AActor* DamageCauser) const
{
	const ASGGameMode* GameMode = Cast<ASGGameMode>(UGameplayStatics::GetGameMode(this));
	if (!IsValid(GameMode)) return false;

	const ASGCharacter* Damager = Cast<ASGCharacter>(DamageCauser);

	if (Damager && (!GameMode->IsFriendlyFireAllowed() && Damager->GetTeam() == GetTeam()))
	{
		return false;
	}

	return Health > 0.f && Super::ShouldTakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

float ASGCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                               AActor* DamageCauser)
{
	const float ActualDamage = FMath::Clamp(DamageAmount, 0.f, Health);
	if (!ShouldTakeDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser)) return 0.f;

	Health -= ActualDamage;
	if (Health == 0.f) AuthDie();

	return ActualDamage;
}

void ASGCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	TargetCameraHeight = CrouchedEyeHeight;
}

void ASGCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	TargetCameraHeight = BaseEyeHeight;
}

ETeam ASGCharacter::GetTeam() const
{
	const UWorld* World = GetWorld();
	if (!IsValid(World)) return ETeam::None;

	const ASGGameState* GameState = World->GetGameState<ASGGameState>();
	if (!IsValid(GameState)) return ETeam::None;

	const ASGPlayerState* Player = GetPlayerState<ASGPlayerState>();
	if (!IsValid(Player)) return ETeam::None;

	return Player->GetTeam();
}

void ASGCharacter::AuthReset(const AActor* PlayerStart)
{
	if (!HasAuthority()) return;

	Health = MaxHealth;
	bIsDead = false;
	
	SetActorLocation(PlayerStart->GetActorLocation());
	MultiResetAnimations();
	MultiSetDeadCollision(false);
}

void ASGCharacter::MultiPlayHitReactMontage_Implementation(const FName& HitBoneName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance) || !IsValid(HitReactMontage)) return;

	AnimInstance->Montage_Play(HitReactMontage);
}

void ASGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASGCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASGCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction<FCrouchDelegate>("Crouch", IE_Pressed, this, &ACharacter::Crouch, false);
	PlayerInputComponent->BindAction<FCrouchDelegate>("Crouch", IE_Released, this, &ACharacter::UnCrouch, false);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASGCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASGCharacter::StopFire);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASGCharacter::Reload);
}

void ASGCharacter::StartFire()
{
	if (!IsValid(Weapon)) return;

	Weapon->ServerStartFire();
}

void ASGCharacter::StopFire()
{
	if (!IsValid(Weapon)) return;

	Weapon->ServerStopFire();
}

void ASGCharacter::Reload()
{
	if (!IsValid(Weapon)) return;

	Weapon->ServerReload();
}

void ASGCharacter::AuthDie()
{
	if (!HasAuthority()) return;

	bIsDead = true;
	OnRep_IsDead();

	ASGPlayerState* DetailedPlayerState = GetPlayerState<ASGPlayerState>();
	
	if (IsValid(DetailedPlayerState))
	{
		DetailedPlayerState->MultiHandleDie();
	}

	MultiSetDeadCollision(true);

	// UWorld* World = GetWorld();
	// if (!IsValid(World)) return;
	//
	// const AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	// if (!IsValid(GameMode)) return;
	//
	// ASpectatorPawn* SpectatorPawn = World->SpawnActor<ASpectatorPawn>(GameMode->SpectatorClass, GetActorLocation(), GetActorRotation());
	// GetController()->Possess(SpectatorPawn);
}

void ASGCharacter::MultiResetAnimations_Implementation()
{
	UAnimInstance* MeshAnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(MeshAnimInstance)) MeshAnimInstance->StopAllMontages(false);
}

void ASGCharacter::MultiSetDeadCollision_Implementation(bool bNewDeadCollision)
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, bNewDeadCollision ? ECR_Ignore : ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, bNewDeadCollision ? ECR_Ignore : ECR_Block);
}

void ASGCharacter::HandleMatchBegin()
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!IsValid(CharacterMesh)) return;

	const ETeam Team = GetTeam();

	if (Team == ETeam::Red)
	{
		CharacterMesh->SetMaterial(0, RedTeamMaterial);
		ArmsMesh->SetMaterial(0, RedTeamMaterial);
	}
	else if (Team == ETeam::Blue)
	{
		CharacterMesh->SetMaterial(0, BlueTeamMaterial);
		ArmsMesh->SetMaterial(0, BlueTeamMaterial);
	}
}

void ASGCharacter::OnRep_IsDead()
{
	if (!bIsDead) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	if (IsValid(AnimInstance) && IsValid(DeathMontage))
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
}
