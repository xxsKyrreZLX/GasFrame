#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "MythosGameplayAbility.generated.h"

class UMythosAttributeSet;
class UAnimMontage;

/**
 * Enum for different types of skills
 * skills have different type of area effected
 */
UENUM(BlueprintType)
enum class EMythosAbilityType : uint8
{
    // no aim - self center
    Self UMETA(DisplayName = "Self"),
    
    // targeted
    Targeted UMETA(DisplayName = "Targeted"),
    
    // non-targeted, directional
    Directional UMETA(DisplayName = "Directional"),
    
    // area - assign location with a radius
    Area UMETA(DisplayName = "Area")
};

/**
 * base class for all the skills
 */
UCLASS()
class MYTHOS_API UMythosGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UMythosGameplayAbility();

    // Activate
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    // End Ability
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

    // type of skill
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability|Type")
    EMythosAbilityType AbilityType;

    // tag container, a series of tags
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability")
    FGameplayTagContainer MythosAbilityTags;

    // CD
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability")
    FScalableFloat CooldownDuration;

    // 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability")
    FScalableFloat CostValue;

    // which type of resources to drain(mana,stamina or even health)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability")
    FGameplayAttribute CostAttribute;

    // animation montage to play
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability")
    UAnimMontage* AbilityMontage;

    // sound effect. maybe we prefer using notify and meta sound?
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability")
    USoundBase* AbilitySound;

    // GE
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability")
    UParticleSystem* AbilityEffect;

    // === area paras ===
    
    // distance - used for Targeted and Directional
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability|Range", meta = (EditCondition = "AbilityType == EMythosAbilityType::Targeted || AbilityType == EMythosAbilityType::Directional"))
    float AbilityDistance;

    // angle - Directional
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability|Range", meta = (EditCondition = "AbilityType == EMythosAbilityType::Directional", ClampMin = "0.0", ClampMax = "360.0"))
    float AbilityAngle;

    // radius - Area
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability|Range", meta = (EditCondition = "AbilityType == EMythosAbilityType::Area", ClampMin = "0.0"))
    float AbilityRadius;

    // self center Radius
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability|Range", meta = (EditCondition = "AbilityType == EMythosAbilityType::Self", ClampMin = "0.0"))
    float SelfEffectRadius;

    // Can use while Moving? QQQ need discussion in this feature
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability")
    bool bCanUseWhileMoving;

    // Can Interrupted?
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability")
    bool bCanBeInterrupted;

    // Rotation speed for smooth character rotation (degrees per second)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mythos|Ability|Rotation", meta = (ClampMin = "1.0", ClampMax = "360.0"))
    float RotationSpeed = 180.0f;

    // === support function ===
    
    // get range of the GA
    UFUNCTION(BlueprintCallable, Category = "Mythos|Ability|Range")
    float GetAbilityRange() const;

    // test if an actor is in the attack range
    UFUNCTION(BlueprintCallable, Category = "Mythos|Ability|Range")
    bool IsTargetInRange(const FVector& CasterLocation, const FVector& TargetLocation, const FVector& CasterForward = FVector::ZeroVector) const;

    // get range of the GE
    UFUNCTION(BlueprintCallable, Category = "Mythos|Ability|Range")
    float GetEffectRadius() const;

    /**
 * Now, GetMouseWorldPosition works well and returns the mouse world location and facing direction.
 *
 * For abilities, I plan to write a helper function to determine valid targets within the effective range of an ability,
 * based on the skill's type and data.
 *
 * The required information should come from the character's own data:
 *
 * - If the ability type is "Area" (with mouse targeting):
 *      Perform a distance check between the mouse position and the character position.
 *      If within ability distance, perform a sphere overlap at the mouse position with the ability radius,
 *      and return all characters that contain the required input tags.
 *
 * - If the ability type is "Area" (centered on the character):
 *      Perform a sphere overlap at the character's position with the ability radius,
 *      and return all characters within the radius.
 *
 * - If the ability type is "Directional":
 *      Use the mouse direction as the center direction,
 *      and perform a cone or sphere check with the specified radius,
 *      returning all characters within the effective range.
 *
 * - If the ability type is "Targeted":
 *      Use the hit result from GetMouseWorldPosition for collision detection,
 *      and return a list containing only the hit character, or an empty list if none is hit.
 */

    UFUNCTION(BlueprintCallable, Category = "Mythos|Ability")
    TArray<AActor*> GetAbilityTargets(FGameplayTag TagFilter);

    // Get mouse direction and position for projectile abilities
    UFUNCTION(BlueprintCallable, Category = "Mythos|Ability")
    bool GetMouseDirectionAndPosition(FVector& OutDirection, FVector& OutPosition) const;

    // Enemy-specific target detection - enemies face the player and attack forward
    UFUNCTION(BlueprintCallable, Category = "Mythos|Ability|Enemy")
    TArray<AActor*> GetEnemyAbilityTargets(FGameplayTag TagFilter);

    // add tag to actor for duration, designed for GE like stun, slow, etc. Cool down may be included.
    UFUNCTION(BlueprintCallable, Category = "Mythos|Ability")
    void AddTagToActorForDuration(AActor* TargetActor, FGameplayTag TagToAdd, float Duration);

    // Make character face mouse position
    UFUNCTION(BlueprintCallable, Category = "Mythos|Ability")
    bool FaceMousePosition();

    // Make character face mouse position with smooth rotation
    UFUNCTION(BlueprintCallable, Category = "Mythos|Ability")
    bool FaceMousePositionSmooth(float CustomRotationSpeed = 10.0f);

    // Start smooth rotation to mouse position using the ability's rotation speed
    UFUNCTION(BlueprintCallable, Category = "Mythos|Ability")
    bool StartSmoothRotationToMouse();

protected:
    UPROPERTY(BlueprintReadOnly, Category="Ability")
    UAbilitySystemComponent* ASC = nullptr;

    void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;

    void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;

    // check if can be used
    bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

    // Interface for Blueprint-friendly versions
    UFUNCTION(BlueprintCallable, Category = "Mythos|Ability")
    void BPApplyCooldown();

    UFUNCTION(BlueprintCallable, Category = "Mythos|Ability")
    void BPApplyCost();

    UFUNCTION(BlueprintCallable, Category = "Mythos|Ability")
    bool BPCheckCost();

protected:
    void PlayAbilityAnimation();

    void PlayAbilitySound();

    void PlayAbilityEffect();

    // get attribute set
    UFUNCTION(BlueprintCallable, Category = "Mythos|Ability")
    const UMythosAttributeSet* GetMythosAttributeSet() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Mythos|Ability")
    void OnAbilityStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Mythos|Ability")
    void OnAbilityEnded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Mythos|Ability")
    void OnAbilityInterrupted();

private:
    // Handke
    FActiveGameplayEffectHandle CooldownEffectHandle;
    FActiveGameplayEffectHandle CostEffectHandle;
};

// === Example Skill Classes ===

/**
 * Fireball Ability - Targeted skill example
 * Requires a specific target, has distance limitation
 */
UCLASS()
class MYTHOS_API UMythosFireballAbility : public UMythosGameplayAbility
{
    GENERATED_BODY()

public:
    UMythosFireballAbility()
    {
        AbilityType = EMythosAbilityType::Targeted;
        AbilityDistance = 500.0f;
        CostValue = FScalableFloat(20.0f); // Consumes 20 mana points
        CooldownDuration = FScalableFloat(3.0f); // 3 second cooldown
        RotationSpeed = 180.0f; // Fast rotation for targeted abilities
    }
};

/**
 * Firewall Ability - Directional skill example
 * Casts in specified direction, has distance and angle limitations
 */
UCLASS()
class MYTHOS_API UMythosFirewallAbility : public UMythosGameplayAbility
{
    GENERATED_BODY()

public:
    UMythosFirewallAbility()
    {
        AbilityType = EMythosAbilityType::Directional;
        AbilityDistance = 300.0f;
        AbilityAngle = 60.0f; // 60 degree cone
        CostValue = FScalableFloat(30.0f);
        CooldownDuration = FScalableFloat(8.0f);
        RotationSpeed = 120.0f; // Medium rotation for directional abilities
    }
};

/**
 * Explosion Ability - Area skill example
 * Explodes at specified location, has radius range
 */
UCLASS()
class MYTHOS_API UMythosExplosionAbility : public UMythosGameplayAbility
{
    GENERATED_BODY()

public:
    UMythosExplosionAbility()
    {
        AbilityType = EMythosAbilityType::Area;
        AbilityRadius = 150.0f;
        CostValue = FScalableFloat(50.0f);
        CooldownDuration = FScalableFloat(15.0f);
        RotationSpeed = 90.0f; // Slower rotation for area abilities
    }
};

/**
 * Healing Aura Ability - Self skill example
 * Healing effect centered on self
 */
UCLASS()
class MYTHOS_API UMythosHealingAuraAbility : public UMythosGameplayAbility
{
    GENERATED_BODY()

public:
    UMythosHealingAuraAbility()
    {
        AbilityType = EMythosAbilityType::Self;
        SelfEffectRadius = 200.0f;
        CostValue = FScalableFloat(40.0f);
        CooldownDuration = FScalableFloat(20.0f);
        RotationSpeed = 60.0f; // Very slow rotation for self abilities
    }
};