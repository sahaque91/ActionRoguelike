// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "Gameframework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "SInteractionComponent.h"
#include "SAttributesComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "SActionComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	TimeToHitParamName = "TimeToHit";
	HandSocketName = "Muzzle_01";

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComp->SetupAttachment(SpringArmComp);

	InteractionComp = CreateDefaultSubobject<USInteractionComponent>(TEXT("InteractionComp"));

	AttributeComp = CreateDefaultSubobject<USAttributesComponent>(TEXT("AttributeComp"));

	ActionComp = CreateDefaultSubobject<USActionComponent>(TEXT("ActionComp"));
}

void ASCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AttributeComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
}

FVector ASCharacter::GetPawnViewLocation() const
{
	return CameraComp->GetComponentLocation();
}

void ASCharacter::OnHealthChanged(AActor* InstigatorActor, USAttributesComponent* OwningComp, float NewHealth, float Delta)
{
	if (Delta < 0.0f)
	{
		GetMesh()->SetScalarParameterValueOnMaterials(TimeToHitParamName, GetWorld()->TimeSeconds);

		// Rage added equal to damage received (Abs to turn into positive rage number)
		float RageDelta = FMath::Abs(Delta);
		AttributeComp->ApplyRage(InstigatorActor, RageDelta);
	}
	if (NewHealth <= 0.0f && Delta < 0.0f)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		DisableInput(PC);

		SetLifeSpan(5.0f);
	}
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked <UEnhancedInputComponent>(PlayerInputComponent))
		{
			EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASCharacter::Move);
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASCharacter::Look);
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASCharacter::PrimaryAttack);
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ASCharacter::PrimaryInteract);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(SpecialAttackAction, ETriggerEvent::Triggered, this, &ASCharacter::SpecialAttack);
			EnhancedInputComponent->BindAction(TeleportAction, ETriggerEvent::Triggered, this, &ASCharacter::Teleport);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ASCharacter::StartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASCharacter::StopSprint);
			EnhancedInputComponent->BindAction(ParryAction, ETriggerEvent::Triggered, this, &ASCharacter::Parry);
		}
	}
}

void ASCharacter::HealSelf(float Amount /* 100 */)
{
	AttributeComp->ApplyHealthChange(this, Amount);
}

void ASCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ASCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void ASCharacter::PrimaryAttack(const FInputActionValue& Value)
{
	ActionComp->StartActionByName(this, "PrimaryAttack");
}

void ASCharacter::StartAttackEffects()
{
	PlayAnimMontage(AttackAnim);

	UGameplayStatics::SpawnEmitterAttached(CastingEffect, GetMesh(), HandSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);
}

FTransform ASCharacter::CalculateProjectileSpawnTransform(FActorSpawnParameters& SpawnParams)
{
	FVector const HandLocation = GetMesh()->GetSocketLocation("Muzzle_01");
		
	FHitResult Hit;
	FCollisionObjectQueryParams ObjectQueryParams;

	FCollisionShape Shape;
	Shape.SetSphere(20.0f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);

	FVector Start = CameraComp->GetComponentLocation();
	FVector End = Start + (GetControlRotation().Vector() * 5000.0f);

	if (GetWorld()->SweepSingleByObjectType(Hit,Start, End, FQuat::Identity, ObjectQueryParams, Shape, QueryParams))
	{
		End = Hit.ImpactPoint;
	}
	FRotator ProjRotation = FRotationMatrix::MakeFromX(End - HandLocation).Rotator();

	return FTransform(ProjRotation, HandLocation);
}

void ASCharacter::PrimaryAttack_TimeElapsed()
{
	if (ensureAlways(ProjectileClass))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = this;

		FTransform const SpawnTM = CalculateProjectileSpawnTransform(SpawnParams);
		GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTM, SpawnParams);
	}
}

void ASCharacter::PrimaryInteract()
{
	if (InteractionComp)
	{
		InteractionComp->PrimaryInteract();
	}
}

void ASCharacter::SpecialAttack_TimeElapsed()
{
	if (ensureAlways(SpecialProjectileClass))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = this;

		FTransform const SpawnTM = CalculateProjectileSpawnTransform(SpawnParams);
		GetWorld()->SpawnActor<AActor>(SpecialProjectileClass, SpawnTM, SpawnParams);
	}
}

void ASCharacter::SpecialAttack(const FInputActionValue& Value)
{

	ActionComp->StartActionByName(this, "SpecialAttack");
}

void ASCharacter::Teleport_TimeElapsed()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Instigator = this;

	FTransform const SpawnTM = CalculateProjectileSpawnTransform(SpawnParams);
	GetWorld()->SpawnActor<AActor>(TeleportProjectileClass, SpawnTM, SpawnParams);
}

void ASCharacter::Teleport(const FInputActionValue& Value)
{
	ActionComp->StartActionByName(this, "Teleport");
}

void ASCharacter::StartSprint()
{
	ActionComp->StartActionByName(this, "Sprint");
}

void ASCharacter::StopSprint()
{
	ActionComp->StopActionByName(this, "Sprint");
}

void ASCharacter::Parry()
{
	ActionComp->StartActionByName(this, "Parry");
}
