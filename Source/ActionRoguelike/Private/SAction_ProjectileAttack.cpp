// Fill out your copyright notice in the Description page of Project Settings.


#include "SAction_ProjectileAttack.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

USAction_ProjectileAttack::USAction_ProjectileAttack()
{
	HandSocketName = "Muzzle_01";
}


void USAction_ProjectileAttack::AttackDelay_Elapsed(ACharacter* InstigatorCharacter)
{
	if (ensureAlways(ProjectileClass))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = InstigatorCharacter;

		FVector const HandLocation = InstigatorCharacter->GetMesh()->GetSocketLocation(HandSocketName);

		FHitResult Hit;
		FCollisionObjectQueryParams ObjectQueryParams;

		FCollisionShape Shape;
		Shape.SetSphere(20.0f);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(InstigatorCharacter);
		ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
		ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
		ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);

		FVector Start = InstigatorCharacter->GetPawnViewLocation();
		FVector End = Start + (InstigatorCharacter->GetControlRotation().Vector() * 5000.0f);

		if (GetWorld()->SweepSingleByObjectType(Hit, Start, End, FQuat::Identity, ObjectQueryParams, Shape, QueryParams))
		{
			End = Hit.ImpactPoint;
		}
		FRotator ProjRotation = FRotationMatrix::MakeFromX(End - HandLocation).Rotator();

		FTransform SpawnTM = FTransform(ProjRotation, HandLocation);
		GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTM, SpawnParams);
	}

	StopAction(InstigatorCharacter);
}

void USAction_ProjectileAttack::StartAction_Implementation(AActor* InstigatorCharacter)
{
	Super::StartAction_Implementation(InstigatorCharacter);

	ACharacter* Character = Cast <ACharacter>(InstigatorCharacter);

	if (Character)
	{
		Character->PlayAnimMontage(AttackAnim);

		UGameplayStatics::SpawnEmitterAttached(CastingEffect, Character->GetMesh(), HandSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);

		if (Character->HasAuthority())
		{
			FTimerHandle TimerHandle_RespawnDelay;

			FTimerDelegate Delegate;
			Delegate.BindUFunction(this, "AttackDelay_Elapsed", Character);
			float RespawnDelay = 2.0f;

			GetWorld()->GetTimerManager().SetTimer(TimerHandle_RespawnDelay, Delegate, RespawnDelay, false);
		}
	}
}


