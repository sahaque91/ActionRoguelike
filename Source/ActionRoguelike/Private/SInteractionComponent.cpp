// Fill out your copyright notice in the Description page of Project Settings.


#include "SInteractionComponent.h"
#include "DrawDebugHelpers.h"
#include "SGameplayInterface.h"
#include "SWorldUserWidget.h"

static TAutoConsoleVariable<bool> CVarDebugDrawInteraction(TEXT("su.InteractionDebugDraw"), 1.0f, TEXT("Enable Debug Lines for Interact Component."), ECVF_Cheat);

// Sets default values for this component's properties
USInteractionComponent::USInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	TraceRadius = 30.0f;
	TraceDistance = 500.0f;
	CollisionChannel = ECC_WorldDynamic;
}


// Called when the game starts
void USInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void USInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APawn* MyPawn = Cast<APawn>(GetOwner());
	if (MyPawn->IsLocallyControlled())
	{
		FindBestInteractable();
	}
}

void USInteractionComponent::FindBestInteractable()
{
	bool bDebugDraw = CVarDebugDrawInteraction.GetValueOnGameThread();

	FCollisionObjectQueryParams ObjectQueryParams;
	AActor* MyOwner = GetOwner();

	FVector EyeLocation;
	FRotator EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	ObjectQueryParams.AddObjectTypesToQuery(CollisionChannel);

	FVector End = EyeLocation + (EyeRotation.Vector() * TraceDistance);

	/*FHitResult Hit;
	bool bBlockHit = GetWorld()->LineTraceSingleByObjectType(Hit,EyeLocation, End, ObjectQueryParams);*/

	TArray<FHitResult> Hits;

	FCollisionShape Shape;
	Shape.SetSphere(TraceRadius);
	bool bBlockHit = GetWorld()->SweepMultiByObjectType(Hits, EyeLocation, End, FQuat::Identity, ObjectQueryParams, Shape);
	FColor LineColor = bBlockHit ? FColor::Green : FColor::Red;

	// Clear ref before trying to fill
	FocusedActor = nullptr;

	for (FHitResult Hit : Hits)
	{

		if (bDebugDraw)
		{
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, TraceRadius, 32, LineColor, false, 0.0f);
		}

		AActor* HitActor = Hit.GetActor();

		if (HitActor)
		{
			if (HitActor->Implements<USGameplayInterface>())
			{
				FocusedActor = HitActor;
				break;
			}
		}
	}

	if (FocusedActor)
	{
		if (DefaultWidgetInstance == nullptr && ensure(DefaultWidgetClass))
		{
			DefaultWidgetInstance = CreateWidget<USWorldUserWidget>(GetWorld(), DefaultWidgetClass);
		}

		if (DefaultWidgetInstance)
		{
			DefaultWidgetInstance->AttachedActor = FocusedActor;

			if (!DefaultWidgetInstance->IsInViewport())
			{
				DefaultWidgetInstance->AddToViewport();
			}
		}
	}
	else
	{
		if (DefaultWidgetInstance)
		{
			DefaultWidgetInstance->RemoveFromParent();
		}
	}

	//if (bDebugDraw)
	//{
	//	DrawDebugLine(GetWorld(), EyeLocation, End, LineColor, false, 2.0f, 0, 2.0f);
	//}
}

void USInteractionComponent::ServerInteract_Implementation(AActor* InFocus)
{
	if (InFocus == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, "No Focus Actor to interact.");
		return;
	}

	APawn* MyPawn = Cast<APawn>(GetOwner());

	ISGameplayInterface::Execute_Interact(InFocus, MyPawn);
}

void USInteractionComponent::PrimaryInteract()
{
	ServerInteract(FocusedActor);
}

