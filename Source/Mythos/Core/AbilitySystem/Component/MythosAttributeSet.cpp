#include "MythosAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemBlueprintLibrary.h"

UMythosAttributeSet::UMythosAttributeSet()
{
    // defaultValue
    InitHealth(100.0f);
    InitMaxHealth(100.0f);
    InitMana(50.0f);
    InitMaxMana(50.0f);
    InitStamina(100.0f);
    InitMaxStamina(100.0f);
    //InitAttackPower(10.0f);
    InitAttackPower(1.0f);//attack as a rate
    InitDefense(0.1f);
    InitMoveSpeed(600.0f);
    InitAttackSpeed(1.0f);
    InitCriticalChance(0.05f);
    InitCriticalDamage(1.5f);
    InitDamage(0.0f);
    InitHealingPower(1.0f);
    InitHealingCriticalChance(0.05f);
    InitHealingCriticalDamage(1.5f);
}

void UMythosAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);
    
    // Debug: Check if this function is being called
    if (GEngine)
    {
        FString DebugMessage = FString::Printf(TEXT("PostGameplayEffectExecute called! Attribute: %s, Magnitude: %.2f"), 
            *Data.EvaluatedData.Attribute.GetName(), Data.EvaluatedData.Magnitude);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, DebugMessage);
    }

    FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
    UAbilitySystemComponent* SourceASC = Context.GetOriginalInstigatorAbilitySystemComponent();
    const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();

    // get Source Actor
    AActor* SourceActor = nullptr;
    if (SourceASC)
    {
        SourceActor = SourceASC->GetOwner();
    }

    // trigger GE applied delegate
    if (SourceActor)
    {
        FString EffectName = Data.EffectSpec.Def ? Data.EffectSpec.Def->GetName() : TEXT("None");
        OnGameplayEffectApplied.Broadcast(SourceActor, EffectName, Data.EvaluatedData.Magnitude);
    }

    // get attributes
    FGameplayAttribute Attribute = Data.EvaluatedData.Attribute;
    float Magnitude = Data.EvaluatedData.Magnitude;
    
    // if damage attribute is applied, apply damage calculation system
    if (Attribute == GetDamageAttribute())
    {
        float RawDamage = GetDamage();
        float FinalDamage = CalculateDamageWithAttributes(Data, RawDamage);
        float NewHealth = FMath::Clamp(GetHealth() - FinalDamage, 0.0f, GetMaxHealth());
        SetHealth(NewHealth);
        SetDamage(0.0f);
        if (GEngine)
        {
            FString Msg = FString::Printf(TEXT("Damage applied: %.2f, Health now: %.2f"), FinalDamage, NewHealth);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, Msg);
        }
        return;
    }

    // Apply complex damage calculations for health changes
    if (Attribute == GetHealthAttribute() && Magnitude < 0.0f)
    {
        
        // This is damage, apply damage calculation system
        float FinalDamage = CalculateDamageWithAttributes(Data, FMath::Abs(Magnitude));

        // Apply the calculated damage (negative value)
        if (FinalDamage != FMath::Abs(Magnitude))
        {
            // Calculate the difference between original and final damage
            float OriginalDamage = FMath::Abs(Magnitude);
            float DamageDifference = -(FinalDamage - OriginalDamage);
            
            // Apply the corrected damage (make it negative for health reduction)
            Health.SetCurrentValue(GetHealth() + DamageDifference);

        }
        else
        {
            // If no damage calculation was applied, show this
            if (GEngine)
            {
                FString NoCorrectionMessage = FString::Printf(TEXT("No damage correction applied - using original: %.1f"), FMath::Abs(Magnitude));
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Silver, NoCorrectionMessage);
            }
        }
    }
    else if (Attribute == GetHealthAttribute())
    {
        // Debug: Show non-damage health changes
        if (GEngine)
        {
            FString HealthDebugMessage = FString::Printf(TEXT("Health change (not damage): %.2f"), Magnitude);
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::White, HealthDebugMessage);
        }
    }
    
    //check they are in the valid range - use direct assignment to avoid triggering PostAttributeChange again
    if (Attribute == GetHealthAttribute())
    {
        float ClampedHealth = FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth());
        if (GetHealth() != ClampedHealth)
        {
            // Direct assignment to avoid triggering PostAttributeChange
            Health.SetCurrentValue(ClampedHealth);
        }
    }
    else if (Attribute == GetManaAttribute())
    {
        float ClampedMana = FMath::Clamp(GetMana(), 0.0f, GetMaxMana());
        if (GetMana() != ClampedMana)
        {
            // Direct assignment to avoid triggering PostAttributeChange
            Mana.SetCurrentValue(ClampedMana);
        }
    }
    else if (Attribute == GetStaminaAttribute())
    {
        float ClampedStamina = FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina());
        if (GetStamina() != ClampedStamina)
        {
            // Direct assignment to avoid triggering PostAttributeChange
            Stamina.SetCurrentValue(ClampedStamina);
        }
    }
}

float UMythosAttributeSet::CalculateDamageWithAttributes(const FGameplayEffectModCallbackData& Data, float BaseDamage)
{
    float FinalDamage = BaseDamage;
    
    // Get source and target info
    FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
    UAbilitySystemComponent* SourceASC = Context.GetOriginalInstigatorAbilitySystemComponent();
    UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent();
    
    if (SourceASC && TargetASC)
    {
        // Get source attributes
        float SourceAttackPower = 1.0f;
        float SourceCriticalChance = 0.05f;
        float SourceCriticalDamage = 1.5f;
        
        if (const UAttributeSet* SourceAttributeSet = SourceASC->GetSet<UMythosAttributeSet>())
        {
            const UMythosAttributeSet* MythosSourceAttributeSet = Cast<UMythosAttributeSet>(SourceAttributeSet);
            if (MythosSourceAttributeSet)
            {
                SourceAttackPower = MythosSourceAttributeSet->GetAttackPower();
                SourceCriticalChance = MythosSourceAttributeSet->GetCriticalChance();
                SourceCriticalDamage = MythosSourceAttributeSet->GetCriticalDamage();
            }
        }
        
        // Get target defense
        float TargetDefense = GetDefense();
        
        // Step 1: Apply AttackPower as damage multiplier
        FinalDamage = BaseDamage * SourceAttackPower;
        
        // Step 2: Apply Defense as damage reduction
        // Defense of 0.1 means 10% damage reduction
        float DefenseReduction = 1.0f - TargetDefense;
        FinalDamage = FinalDamage * DefenseReduction;
        
        // Step 3: Apply Critical Chance and Critical Damage
        float RandomValue = FMath::FRand();
        if (RandomValue < SourceCriticalChance)
        {
            // Critical hit! Apply critical damage multiplier
            FinalDamage = FinalDamage * SourceCriticalDamage;
            
            // You could trigger a critical hit event here
            // OnCriticalHit.Broadcast(FinalDamage);
            
            if (GEngine)
            {
                FString CriticalMessage = FString::Printf(TEXT("CRITICAL HIT! Damage: %.1f"), FinalDamage);
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, CriticalMessage);
            }
        }
    }
    
    return FinalDamage;
}

void UMythosAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    // check again
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
    else if (Attribute == GetManaAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
    }
    else if (Attribute == GetStaminaAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
    }
    else if (Attribute == GetCriticalChanceAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
    }
    else if (Attribute == GetCriticalDamageAttribute())
    {
        NewValue = FMath::Max(NewValue, 1.0f);
    }
    else if (Attribute == GetHealingPowerAttribute())
    {
        NewValue = FMath::Max(NewValue, 0.0f);
    }
    else if (Attribute == GetHealingCriticalChanceAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
    }
    else if (Attribute == GetHealingCriticalDamageAttribute())
    {
        NewValue = FMath::Max(NewValue, 1.0f);
    }
}

void UMythosAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);

    // Post attribute change
    if (Attribute == GetHealthAttribute())
    {
        OnHealthChanged.Broadcast(OldValue, NewValue, Attribute);
    }
    else if (Attribute == GetManaAttribute())
    {
        OnManaChanged.Broadcast(OldValue, NewValue,  Attribute);
    }
    else if (Attribute == GetStaminaAttribute())
    {
        OnStaminaChanged.Broadcast(OldValue, NewValue, Attribute);
    }
}