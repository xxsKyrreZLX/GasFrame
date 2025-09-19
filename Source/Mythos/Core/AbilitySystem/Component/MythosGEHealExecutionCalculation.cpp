// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AbilitySystem/Component/MythosGEHealExecutionCalculation.h"
#include "Core/AbilitySystem/Component/MythosAttributeSet.h"
#include "Engine/Engine.h"

struct FMythosHealStatics
{
    FGameplayEffectAttributeCaptureDefinition HealDef;
    FGameplayEffectAttributeCaptureDefinition HealingPowerDef;
    FGameplayEffectAttributeCaptureDefinition HealingCritChanceDef;
    FGameplayEffectAttributeCaptureDefinition HealingCritDamageDef;
    FMythosHealStatics()
        : HealDef(UMythosAttributeSet::GetDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true) // 使用Damage作为基础治疗值
        , HealingPowerDef(UMythosAttributeSet::GetHealingPowerAttribute(), EGameplayEffectAttributeCaptureSource::Source, false)
        , HealingCritChanceDef(UMythosAttributeSet::GetHealingCriticalChanceAttribute(), EGameplayEffectAttributeCaptureSource::Source, false)
        , HealingCritDamageDef(UMythosAttributeSet::GetHealingCriticalDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, false)
    {}
};
static FMythosHealStatics& HealStatics()
{
    static FMythosHealStatics Statics;
    return Statics;
}

UMythosGEHealExecutionCalculation::UMythosGEHealExecutionCalculation()
{
    RelevantAttributesToCapture.Add(HealStatics().HealDef);
    RelevantAttributesToCapture.Add(HealStatics().HealingPowerDef);
    RelevantAttributesToCapture.Add(HealStatics().HealingCritChanceDef);
    RelevantAttributesToCapture.Add(HealStatics().HealingCritDamageDef);
}

void UMythosGEHealExecutionCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    float Heal = 0.f;
    float HealingPower = 1.f;
    float HealingCritChance = 0.05f;
    float HealingCritDamage = 1.5f;

    FAggregatorEvaluateParameters EvalParams;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HealStatics().HealDef, EvalParams, Heal);
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HealStatics().HealingPowerDef, EvalParams, HealingPower);
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HealStatics().HealingCritChanceDef, EvalParams, HealingCritChance);
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HealStatics().HealingCritDamageDef, EvalParams, HealingCritDamage);

    // 治疗结算
    float FinalHeal = Heal * HealingPower;
    bool bIsCrit = FMath::FRand() < HealingCritChance;
    if (bIsCrit)
    {
        FinalHeal *= HealingCritDamage;
    }

    if (FinalHeal > 0.f)
    {
        OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
            UMythosAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, FinalHeal));
    }

    // Debug
    if (GEngine)
    {
        FString Msg = FString::Printf(TEXT("Heal: %.1f%s"), FinalHeal, bIsCrit ? TEXT(" (CRIT)") : TEXT(""));
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, bIsCrit ? FColor::Blue : FColor::Cyan, Msg);
    }
}

