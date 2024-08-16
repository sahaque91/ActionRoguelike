// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerupActor_HealthPotion.h"
#include "SAttributesComponent.h"
#include "ASPlayerState.h"

ASPowerupActor_HealthPotion::ASPowerupActor_HealthPotion()
{
	CreditCost = 50;
}


void ASPowerupActor_HealthPotion::Interact_Implementation(APawn* InstigatorPawn)
{
	if (!ensure(InstigatorPawn))
	{
		return;
	}

	USAttributesComponent* AttributesComp = Cast<USAttributesComponent>(InstigatorPawn->GetComponentByClass(USAttributesComponent::StaticClass()));
	// Check if not already at max health
	if (ensure(AttributesComp) && !AttributesComp->IsFullHealth())
	{
		// Only activate if healed successfully
		if (ASPlayerState* PS = InstigatorPawn->GetPlayerState<ASPlayerState>())
		{
			if (PS->RemoveCredits(CreditCost) && AttributesComp->ApplyHealthChange(this, AttributesComp->GetHealthMax()))
			{
				// Only activate if healed successfully
				HideAndCooldownPowerup();
			}
		}
	}
}
