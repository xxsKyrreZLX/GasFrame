// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/AbilitySystem/Component/MythosGEExecutionCalculation.h"
#include "AbilitySystemComponent.h"
#include "Core/AbilitySystem/Component/MythosAttributeSet.h"
#include "GameplayTagContainer.h"
#include "Engine/Engine.h"


struct FMythosDamageStatics
{
    FGameplayEffectAttributeCaptureDefinition DamageDef;
    FGameplayEffectAttributeCaptureDefinition AttackPowerDef;
    FGameplayEffectAttributeCaptureDefinition DefenseDef;
    FGameplayEffectAttributeCaptureDefinition CritChanceDef;
    FGameplayEffectAttributeCaptureDefinition CritDamageDef;
    FMythosDamageStatics()
        : DamageDef(UMythosAttributeSet::GetDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true)
        , AttackPowerDef(UMythosAttributeSet::GetAttackPowerAttribute(), EGameplayEffectAttributeCaptureSource::Source, false)
        , DefenseDef(UMythosAttributeSet::GetDefenseAttribute(), EGameplayEffectAttributeCaptureSource::Target, false)
        , CritChanceDef(UMythosAttributeSet::GetCriticalChanceAttribute(), EGameplayEffectAttributeCaptureSource::Source, false)
        , CritDamageDef(UMythosAttributeSet::GetCriticalDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, false)
    {}
};
static FMythosDamageStatics& DamageStatics()
{
    static FMythosDamageStatics Statics;
    return Statics;
}

UMythosGEExecutionCalculation::UMythosGEExecutionCalculation()
{
    RelevantAttributesToCapture.Add(DamageStatics().DamageDef);
    RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
    RelevantAttributesToCapture.Add(DamageStatics().DefenseDef);
    RelevantAttributesToCapture.Add(DamageStatics().CritChanceDef);
    RelevantAttributesToCapture.Add(DamageStatics().CritDamageDef);
}

void UMythosGEExecutionCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    float Damage = 0.f;
    float AttackPower = 1.f;
    float Defense = 0.f;
    float CritChance = 0.5f;
    float CritDamage = 1.5f;

    FAggregatorEvaluateParameters EvalParams;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DamageDef, EvalParams, Damage);
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvalParams, AttackPower);
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenseDef, EvalParams, Defense);
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritChanceDef, EvalParams, CritChance);
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritDamageDef, EvalParams, CritDamage);

    // Damage Calculation
    float FinalDamage = Damage * AttackPower * (1.0f - Defense);
    bool bIsCrit = FMath::FRand() < CritChance;
    if (bIsCrit)
    {
        FinalDamage *= CritDamage;
    }

    if (FinalDamage > 0.f)
    {
        OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
            UMythosAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -FinalDamage));
    }

    // Debug
    if (GEngine)
    {
        FString Msg = FString::Printf(TEXT("Damage!!: %.1f%s"), FinalDamage, bIsCrit ? TEXT(" (CRIT)") : TEXT(""));
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, bIsCrit ? FColor::Red : FColor::Green, Msg);
    }
}

