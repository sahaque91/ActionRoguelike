// Fill out your copyright notice in the Description page of Project Settings.


#include "STeleportProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASTeleportProjectile::ASTeleportProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASTeleportProjectile::BeginPlay()
{
	Super::BeginPlay();

	SphereComp->IgnoreActorWhenMoving(GetInstigator(), true);
	GetWorldTimerManager().SetTimer(TimerHandle_DelayExplode, this, &ASTeleportProjectile::Teleport, .2f);
	
}

void ASTeleportProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SphereComp->OnComponentHit.AddDynamic(this, &ASTeleportProjectile::OnComponentHit);
}

void ASTeleportProjectile::TeleportPlayer()
{
	if (AActor* Player = GetInstigator())
	{
		Player->TeleportTo(GetActorLocation(), Player->GetActorRotation());
	}
	Destroy();
}

void ASTeleportProjectile::Teleport()
{
	GetWorldTimerManager().ClearTimer((TimerHandle_DelayExplode));
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DetonateEffect, GetActorLocation(), GetActorRotation());
	MovementComp->StopMovementImmediately();
	GetWorldTimerManager().SetTimer(TimerHandle_DelayTeleport, this, &ASTeleportProjectile::TeleportPlayer, .2f);
	TeleportPlayer();
}

void ASTeleportProjectile::OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	
	
}

// Called every frame
void ASTeleportProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

