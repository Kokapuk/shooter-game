#include "SGCharacter.h"

#include "SGAbilityComponent.h"
#include "SGAbilityDataAsset.h"
#include "SGBlindnessComponent.h"
#include "SGCharacterMovementComponent.h"
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
#include "GameFramework/SpectatorPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

ASGCharacter::ASGCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USGCharacterMovementComponent>(CharacterMovementComponentName))
{
	MaxHealth = 100.f;
	BaseEyeHeight = 166.f;
	CrouchedEyeHeight = 115.f;
	TargetCameraHeight = BaseEyeHeight;

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	Capsule->SetCapsuleHalfHeight(96.f);
	Capsule->SetCapsuleRadius(55.f);

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	CharacterMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	CharacterMesh->SetRelativeLocation(FVector(0.f, 0.f, -Capsule->GetScaledCapsuleHalfHeight()));
	CharacterMesh->SetOwnerNoSee(true);
	CharacterMesh->SetGenerateOverlapEvents(true);

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(CharacterMesh);
	Camera->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	Camera->SetRelativeScale3D(FVector(.5f));
	Camera->SetFieldOfView(103.f);
	Camera->bUsePawnControlRotation = true;

	ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ArmsMesh");
	ArmsMesh->SetOnlyOwnerSee(true);
	ArmsMesh->SetRelativeLocation(FVector(0.f, 10.f, -160.f));
	ArmsMesh->SetupAttachment(Camera);
	ArmsMesh->bCastDynamicShadow = false;
	ArmsMesh->CastShadow = false;

	FirstPersonWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("FirstPersonGunMesh");
	FirstPersonWeaponMesh->SetupAttachment(ArmsMesh);
	FirstPersonWeaponMesh->SetOnlyOwnerSee(true);
	FirstPersonWeaponMesh->bCastDynamicShadow = false;
	FirstPersonWeaponMesh->CastShadow = false;

	ThirdPersonWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ThirdPersonGunMesh");
	ThirdPersonWeaponMesh->SetOwnerNoSee(true);
	ThirdPersonWeaponMesh->SetupAttachment(GetMesh());

	Weapon = CreateDefaultSubobject<USGWeaponComponent>("Weapon");

	BlindnessComponent = CreateDefaultSubobject<USGBlindnessComponent>("BlindnessComponent");
}

void ASGCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Camera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight));
	FirstPersonWeaponMesh->AttachToComponent(ArmsMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
	                                         "GripPoint");
	ThirdPersonWeaponMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
	                                         "GripPoint");
}

void ASGCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGCharacter, AbilityComponent);
	DOREPLIFETIME(ASGCharacter, Health);
	DOREPLIFETIME(ASGCharacter, bIsDead);
	DOREPLIFETIME(ASGCharacter, Team);
}

void ASGCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
}

void ASGCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const FVector NewCameraLocation = UKismetMathLibrary::VInterpTo(Camera->GetRelativeLocation(),
	                                                                FVector(0.f, 0.f, TargetCameraHeight), DeltaSeconds,
	                                                                10.f);
	Camera->SetRelativeLocation(NewCameraLocation);
}

void ASGCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	const ASGPlayerState* DetailedPlayerState = NewController->GetPlayerState<ASGPlayerState>();
	check(IsValid(DetailedPlayerState))

	Team = DetailedPlayerState->GetTeam();
	OnRep_Team();
}

bool ASGCharacter::ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                    AActor* DamageCauser) const
{
	const ASGGameMode* GameMode = GetWorld()->GetAuthGameMode<ASGGameMode>();
	check(IsValid(GameMode))

	const ASGCharacter* Damager = Cast<ASGCharacter>(DamageCauser);
	check(IsValid(Damager))

	if (!GameMode->IsFriendlyFireAllowed() && Damager->GetTeam() == GetTeam())
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

	if (Health == 0.f)
	{
		AuthDie(DamageCauser, DamageAmount == 100.f);
	}

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

void ASGCharacter::ServerSetAbility_Implementation(const USGAbilityDataAsset* NewAbility)
{
	if (!IsValid(NewAbility))
	{
		if (IsValid(AbilityComponent)) AbilityComponent->DestroyComponent();
		return;
	}

	if (IsValid(AbilityComponent)) AbilityComponent->DestroyComponent();

	check(IsValid(NewAbility->AbilityClass));

	AbilityComponent = NewObject<USGAbilityComponent>(this, NewAbility->AbilityClass);
	AbilityComponent->RegisterComponent();
}

ETeam ASGCharacter::GetTeam() const
{
	return Team;
}

void ASGCharacter::AuthReset(const AActor* PlayerStart)
{
	if (!HasAuthority()) return;

	Health = MaxHealth;
	bIsDead = false;

	SetActorLocation(PlayerStart->GetActorLocation());
	MultiReset();

	check(IsValid(Weapon))
	Weapon->AuthReset();

	check(IsValid(AbilityComponent))
	AbilityComponent->AuthReset();

	USGCharacterMovementComponent* DetailedCharacterMovement = Cast<USGCharacterMovementComponent>(
		GetCharacterMovement());
	check(IsValid(DetailedCharacterMovement))

	DetailedCharacterMovement->AuthReset();
}

void ASGCharacter::MultiPlayHitReactMontage_Implementation(const FName& HitBoneName)
{
	check(IsValid(HitReactMontage))

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance)) return;

	AnimInstance->Montage_Play(HitReactMontage);
}

void ASGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAxis("MoveForward", this, &ASGCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASGCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction<FCrouchDelegate>("Crouch", IE_Pressed, this, &ACharacter::Crouch, false);
	PlayerInputComponent->BindAction<FCrouchDelegate>("Crouch", IE_Released, this, &ACharacter::UnCrouch, false);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASGCharacter::Fire);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASGCharacter::Reload);

	PlayerInputComponent->BindAction("UtilizeAbility", IE_Pressed, this, &ASGCharacter::UtilizeAbility);
}

void ASGCharacter::UtilizeAbility()
{
	if (!IsValid(AbilityComponent) || !AbilityComponent->CanBeUtilized()) return;

	AbilityComponent->Utilize();
}

void ASGCharacter::AuthDie(AActor* Killer, const bool bIsHeadshot)
{
	if (!HasAuthority()) return;

	bIsDead = true;
	OnRep_IsDead();

	ASGPlayerState* DetailedPlayerState = GetPlayerState<ASGPlayerState>();
	check(IsValid(DetailedPlayerState));

	DetailedPlayerState->MultiHandleDie();

	GetMovementComponent()->Velocity = FVector::ZeroVector;

	UWorld* World = GetWorld();
	const AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();

	ASpectatorPawn* SpectatorPawn = World->SpawnActor<ASpectatorPawn>(GameMode->SpectatorClass,
	                                                                  Camera->GetComponentLocation(),
	                                                                  GetActorRotation());
	GetController()->Possess(SpectatorPawn);

	DetailedPlayerState->SetIsSpectator(true);

	const APawn* KillerPawn = Cast<APawn>(Killer);
	check(IsValid(KillerPawn))

	ASGPlayerState* KillerPlayerState = KillerPawn->GetPlayerState<ASGPlayerState>();
	check(IsValid(KillerPlayerState))

	KillerPlayerState->AuthHandleKill();

	ASGGameState* GameState = GetWorld()->GetGameState<ASGGameState>();
	check(IsValid(GameState))

	GameState->MultiHandleKill(KillerPlayerState, DetailedPlayerState, bIsHeadshot);
}

void ASGCharacter::MultiReset_Implementation()
{
	SetDeadCollision(false);
	ResetAnimations();

	check(IsValid(BlindnessComponent))
	BlindnessComponent->MultiReset();
}

void ASGCharacter::ResetAnimations() const
{
	UAnimInstance* MeshAnimInstance = GetMesh()->GetAnimInstance();
	check(IsValid(MeshAnimInstance))

	MeshAnimInstance->StopAllMontages(false);

	if (IsLocallyControlled())
	{
		UAnimInstance* ArmsMeshAnimInstance = ArmsMesh->GetAnimInstance();
		UAnimInstance* WeaponAnimInstance = FirstPersonWeaponMesh->GetAnimInstance();
		check(IsValid(ArmsMeshAnimInstance))
		check(IsValid(WeaponAnimInstance))

		ArmsMeshAnimInstance->StopAllMontages(false);
		WeaponAnimInstance->StopAllMontages(false);
	}
	else
	{
		UAnimInstance* WeaponAnimInstance = ThirdPersonWeaponMesh->GetAnimInstance();
		check(IsValid(WeaponAnimInstance))

		WeaponAnimInstance->StopAllMontages(false);
	}
}

void ASGCharacter::SetDeadCollision(const bool bNewDeadCollision) const
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, bNewDeadCollision ? ECR_Ignore : ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, bNewDeadCollision ? ECR_Ignore : ECR_Block);
}

void ASGCharacter::OnRep_IsDead()
{
	if (!bIsDead) return;

	OnDie.Broadcast();
	SetDeadCollision(true);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	check(IsValid(AnimInstance))
	check(IsValid(DeathMontage))

	AnimInstance->Montage_Play(DeathMontage);
}

void ASGCharacter::OnRep_Team()
{
	check(IsValid(RedTeamMaterial))
	check(IsValid(BlueTeamMaterial))

	USkeletalMeshComponent* CharacterMesh = GetMesh();

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
