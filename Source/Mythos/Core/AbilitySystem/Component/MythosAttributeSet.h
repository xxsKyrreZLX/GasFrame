#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "MythosAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * attribute set of base actor in combat system
 * 
 */
UCLASS()
class MYTHOS_API UMythosAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UMythosAttributeSet();

    // call after GE
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

    // call before attribute change
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

    // call after attribute change
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

    // Hp
    UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, Health)

        // maxHP
        UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, MaxHealth)

        // Mana
        UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData Mana;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, Mana)

        // MaxMana
        UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData MaxMana;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, MaxMana)

        // Stamina
        UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData Stamina;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, Stamina)

        // MaxStamina
        UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData MaxStamina;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, MaxStamina)

        // basse attack * attack power, base attack will be defined by attack skills
        UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData AttackPower;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, AttackPower)

        // defense (QQQ we use - or /?)
        UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData Defense;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, Defense)

        // 
        UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData MoveSpeed;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, MoveSpeed)

        // AttackSpeed
        UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData AttackSpeed;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, AttackSpeed)

        // CriticalChance
        UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData CriticalChance;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, CriticalChance)

        // CriticalDamage
        UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData CriticalDamage;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, CriticalDamage)

    // Damage
    UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData Damage;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, Damage)

    // HealingPower - 治疗力
    UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData HealingPower;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, HealingPower)

    // HealingCriticalChance - 治疗暴击率
    UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData HealingCriticalChance;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, HealingCriticalChance)

    // HealingCriticalDamage - 治疗暴击倍率
    UPROPERTY(BlueprintReadOnly, Category = "Mythos|Attributes")
    FGameplayAttributeData HealingCriticalDamage;
    ATTRIBUTE_ACCESSORS(UMythosAttributeSet, HealingCriticalDamage)

    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChangedDelegate, float, OldHealth, float, NewHealth,  const FGameplayAttribute&, Attribute);
    UPROPERTY(BlueprintAssignable, Category = "Mythos|Attributes")
    FOnHealthChangedDelegate OnHealthChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnManaChangedDelegate, float, OldMana, float, NewMana, const FGameplayAttribute&, Attribute);
    UPROPERTY(BlueprintAssignable, Category = "Mythos|Attributes")
    FOnManaChangedDelegate OnManaChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStaminaChangedDelegate, float, OldStamina, float, NewStamina, const FGameplayAttribute&, Attribute);
    UPROPERTY(BlueprintAssignable, Category = "Mythos|Attributes")
    FOnStaminaChangedDelegate OnStaminaChanged;

    // GE applied delegate
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGameplayEffectAppliedDelegate, AActor*, Source, FString, EffectName, float, Magnitude);
    UPROPERTY(BlueprintAssignable, Category = "Mythos|Attributes")
    FOnGameplayEffectAppliedDelegate OnGameplayEffectApplied;

protected:
    // Complex damage calculation with attributes
    float CalculateDamageWithAttributes(const FGameplayEffectModCallbackData& Data, float BaseDamage);
};