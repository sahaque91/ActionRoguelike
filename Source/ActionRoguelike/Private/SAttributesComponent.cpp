// Fill out your copyright notice in the Description page of Project Settings.


#include "SAttributesComponent.h"
#include "SGameModeBase.h"
#include "Net/UnrealNetwork.h"

static TAutoConsoleVariable<float> CVarDamageMultiplier(TEXT("su.DamageMultiplier"), 1.0f, TEXT("Global Damage Modifier for Attribute Component."), ECVF_Cheat);

bool USAttributesComponent::Kill(AActor* InstigatorActor)
{
	return ApplyHealthChange(InstigatorActor, -GetHealthMax());
}

USAttributesComponent* USAttributesComponent::GetAttributes(AActor* FromActor)
{
	if (FromActor)
	{
		return FromActor->FindComponentByClass<USAttributesComponent>();
	}

	return nullptr;
}

bool USAttributesComponent::IsActorAlive(AActor* Actor)
{
	USAttributesComponent* AttributeComp = GetAttributes(Actor);
	if (AttributeComp)
	{
		return AttributeComp->IsAlive();
	}
	return false;
}

// Sets default values for this component's properties
USAttributesComponent::USAttributesComponent()
{
	HealthMax = 100;
	Health = HealthMax;

	Rage = 0;
	RageMax = 100;

	SetIsReplicatedByDefault(true);
}


bool USAttributesComponent::IsAlive() const
{
	return Health > 0.f;
}

bool USAttributesComponent::ApplyHealthChange(AActor* InstigatorActor, float Delta)
{
	if (!GetOwner()->CanBeDamaged() && Delta < 0.0f)
	{
		return false;
	}


	if (Delta < 0.0f)
	{
		float DamageMultiplier = CVarDamageMultiplier.GetValueOnGameThread();

		Delta *= DamageMultiplier;
	}

	float OldHealth = Health;
	float NewHealth = FMath::Clamp(Health + Delta, 0.0f, HealthMax);
	float ActualDelta = NewHealth - OldHealth;

	//Is Server?
	if (GetOwner()->HasAuthority())
	{
		Health = NewHealth;

		if (ActualDelta != 0.0f)
		{
			MulticastHealthChanged(InstigatorActor, Health, ActualDelta);
		}

		// Died
		if (ActualDelta < 0.0f && Health == 0.0f)
		{
			ASGameModeBase* GM = Cast<ASGameModeBase>(GetWorld()->GetAuthGameMode<ASGameModeBase>());
			if (GM)
			{
				GM->OnActorKilled(GetOwner(), InstigatorActor);
			}
		}
	}

	return ActualDelta != 0;
}

bool USAttributesComponent::IsFullHealth() const
{
	return Health == HealthMax;
}


float USAttributesComponent::GetHealthMax() const
{
	return HealthMax;
}

float USAttributesComponent::GetHealth() const
{
	return Health;
}

float USAttributesComponent::GetRage() const
{
	return Rage;
}


bool USAttributesComponent::ApplyRage(AActor* InstigatorActor, float Delta)
{
	float OldRage = Rage;

	Rage = FMath::Clamp(Rage + Delta, 0.0f, RageMax);

	float ActualDelta = Rage - OldRage;
	if (ActualDelta != 0.0f)
	{
		OnRageChanged.Broadcast(InstigatorActor, this, Rage, ActualDelta);
	}

	return ActualDelta != 0;
}


void USAttributesComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USAttributesComponent, Health);
	DOREPLIFETIME(USAttributesComponent, HealthMax);

	//DOREPLIFETIME_CONDITION(USAttributeComponent, HealthMax, COND_OwnerOnly)
}

void USAttributesComponent::MulticastHealthChanged_Implementation(AActor* InstigatorActor, float NewHealth, float Delta)
{
	OnHealthChanged.Broadcast(InstigatorActor, this, NewHealth, Delta);
}
