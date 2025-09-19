// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "MythosGEHealExecutionCalculation.generated.h"

/**
 * 
 */
UCLASS()
class MYTHOS_API UMythosGEHealExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
    UMythosGEHealExecutionCalculation();
    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
