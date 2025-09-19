// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "MythosGEExecutionCalculation.generated.h"

/**
 * 
 */
UCLASS()
class MYTHOS_API UMythosGEExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
    UMythosGEExecutionCalculation();
    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
