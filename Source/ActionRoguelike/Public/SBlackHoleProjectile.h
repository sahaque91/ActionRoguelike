// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SProjectile.h"
#include "GameFramework/Actor.h"
#include "SBlackHoleProjectile.generated.h"

class URadialForceComponent;

UCLASS()
class ACTIONROGUELIKE_API ASBlackHoleProjectile : public ASProjectile
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASBlackHoleProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	URadialForceComponent* RadialForceComp;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
