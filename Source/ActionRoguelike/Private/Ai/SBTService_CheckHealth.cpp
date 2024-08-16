// Fill out your copyright notice in the Description page of Project Settings.


#include "Ai/SBTService_CheckHealth.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "SAttributesComponent.h"

USBTService_CheckHealth::USBTService_CheckHealth()
{
	LowHealthFraction = 0.3f;
}

void USBTService_CheckHealth::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (ensure(AIPawn))
	{
		USAttributesComponent* AttributeComp = USAttributesComponent::GetAttributes(AIPawn);

		UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

		if (ensure(AttributeComp))
		{
			bool bLowHealth = (AttributeComp->GetHealth() / AttributeComp->GetHealthMax()) <= LowHealthFraction;

			BlackboardComp->SetValueAsBool(LowHealthKey.SelectedKeyName, bLowHealth);
		}
	}
}
