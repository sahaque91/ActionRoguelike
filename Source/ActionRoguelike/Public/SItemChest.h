// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGameplayInterface.h"
#include "GameFramework/Actor.h"
#include "ActionRoguelike/Public/SGameplayInterface.h"
#include "SItemChest.generated.h"

class UStaticMeshComponent;

UCLASS()
class ACTIONROGUELIKE_API ASItemChest : public AActor, public ISGameplayInterface
{
	GENERATED_BODY()

	void Interact_Implementation(APawn* InstigatorPawn) override;
	
public:	
	// Sets default values for this actor's properties
	ASItemChest();

	UPROPERTY(EditAnywhere)
	float TargetPitch;

	void OnActorLoaded_Implementation();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing="OnRep_LidOpened", BlueprintReadOnly, SaveGame) //RepNotify
	bool bLidOpened;

	UFUNCTION()
	void OnRep_LidOpened();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* LidMesh;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
