// Fill out your copyright notice in the Description page of Project Settings.


#include "Ai/SBTTask_HealSelf.h"
#include "AIController.h"
#include "SAttributesComponent.h"

EBTNodeResult::Type USBTTask_HealSelf::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* MyController = OwnerComp.GetAIOwner();

	if (ensure(MyController))
	{
		APawn* MyPawn = MyController->GetPawn();
		if (MyPawn == nullptr)
		{
			return EBTNodeResult::Failed;
		}

		USAttributesComponent* AttributeComp = USAttributesComponent::GetAttributes(MyPawn);
		AttributeComp->ApplyHealthChange(MyPawn, AttributeComp->GetHealthMax());
	}
	
	return EBTNodeResult::Succeeded;
}
