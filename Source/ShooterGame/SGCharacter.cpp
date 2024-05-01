#include "SGCharacter.h"

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

float ASGCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                               AActor* DamageCauser)
{
	if (!ShouldTakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser)) return 0.f;
	Health -= DamageAmount;

	return DamageAmount;
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

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASGCharacter::Fire);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASGCharacter::Reload);
}

void ASGCharacter::Fire()
{
	if (!IsValid(Weapon)) return;

	Weapon->Fire();
}

void ASGCharacter::Reload()
{
	if (!IsValid(Weapon)) return;

	Weapon->Reload();
}
