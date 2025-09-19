#include "MythosGameplayAbility.h"
#include "Component/MythosAttributeSet.h"
#include "Component/MythosAbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Animation/AnimMontage.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Engine/Engine.h"
#include "Mythos/MythosPlayerController.h"
#include "Mythos/MythosCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameplayTagAssetInterface.h"
#include "DrawDebugHelpers.h"
#include "MythosCharacter.h"
#include "Abilities/GameplayAbility.h"

UMythosGameplayAbility::UMythosGameplayAbility()
{
    // Default values
    AbilityType = EMythosAbilityType::Self;
    bCanUseWhileMoving = true;
    bCanBeInterrupted = true;
    
    // Skill range parameter default values
    AbilityDistance = 200.0f;
    AbilityAngle = 90.0f;
    AbilityRadius = 100.0f;
    SelfEffectRadius = 50.0f;

    // Default CostAttribute is empty
    CostAttribute = FGameplayAttribute();
}

void UMythosGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    // Call parent class activation method
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    //UE_LOG(LogTemp, Warning, TEXT("CheckCost called: CostValue=%.2f, CostAttribute=%s"), CostValue.GetValue(), *CostAttribute.GetName());

    // get ASC from avatar actor
    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (Avatar)
    {
        AMythosCharacter* MC = Cast<AMythosCharacter>(Avatar);
        if (MC)
        {
            ASC = MC->GetAbilitySystemComponent();
        }
        else
        {
            ASC = nullptr;
        }
    }
    else
    {
        ASC = nullptr;
    }

    // Check cost
    if (!CheckCost(Handle, ActorInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Apply cost
    ApplyCost(Handle, ActorInfo, ActivationInfo);

    // Apply cooldown
    ApplyCooldown(Handle, ActorInfo, ActivationInfo);

    // Start smooth rotation to mouse position
    StartSmoothRotationToMouse();

    // Play skill effects
    PlayAbilityAnimation();
    PlayAbilitySound();
    PlayAbilityEffect();

    // Trigger blueprint events
    OnAbilityStarted();
}

void UMythosGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (bWasCancelled)
    {
        OnAbilityInterrupted();
    }
    else
    {
        OnAbilityEnded();
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMythosGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
    if (CooldownDuration.GetValue() > 0.0f)
    {
        // Create Cooldown and apply
        UGameplayEffect* CooldownEffect = NewObject<UGameplayEffect>();
        CooldownEffect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
        CooldownEffect->Period = 0.0f;
        CooldownEffect->DurationMagnitude = FScalableFloat(CooldownDuration.GetValue());

        if (UAbilitySystemComponent* LocalASC = ActorInfo->AbilitySystemComponent.Get())
        {
            const_cast<UMythosGameplayAbility*>(this)->CooldownEffectHandle = LocalASC->ApplyGameplayEffectToSelf(CooldownEffect, 1.0f, LocalASC->MakeEffectContext());
        }
    }
}

void UMythosGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
    if (CostValue.GetValue() > 0.0f && CostAttribute.IsValid())
    {
        // Creat GE
        UGameplayEffect* CostEffect = NewObject<UGameplayEffect>();
        CostEffect->DurationPolicy = EGameplayEffectDurationType::Instant;

        // Set modifier for GE
        FGameplayModifierInfo CostModifier;
        CostModifier.Attribute = CostAttribute;
        CostModifier.ModifierOp = EGameplayModOp::Additive;
        CostModifier.ModifierMagnitude = FScalableFloat(-CostValue.GetValue());
        CostEffect->Modifiers.Add(CostModifier);

        // Apply GE
        if (UAbilitySystemComponent* LocalASC = ActorInfo->AbilitySystemComponent.Get())
        {
            const_cast<UMythosGameplayAbility*>(this)->CostEffectHandle = LocalASC->ApplyGameplayEffectToSelf(CostEffect, 1.0f, LocalASC->MakeEffectContext());
        }
    }
}

UFUNCTION(BlueprintCallable, Category="Ability")
bool UMythosGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    //UE_LOG(LogTemp, Warning, TEXT("CheckCost called: CostValue=%.2f, CostAttribute=%s"), CostValue.GetValue(), *CostAttribute.GetName());
    if (CostValue.GetValue() <= 0.0f || !CostAttribute.IsValid())
    {
        return true;
    }

    if (UAbilitySystemComponent* LocalASC = ActorInfo->AbilitySystemComponent.Get())
    {
        float CurrentValue = LocalASC->GetNumericAttribute(CostAttribute);
        return CurrentValue >= CostValue.GetValue();
    }

    return false;
}

void UMythosGameplayAbility::PlayAbilityAnimation()
{
    if (AbilityMontage && GetOwningActorFromActorInfo())
    {
        // if we need do something here
        //we can use default function in 
    }
}

void UMythosGameplayAbility::PlayAbilitySound()
{
    if (AbilitySound && GetOwningActorFromActorInfo())
    {
        // sound logic, 
    }
}

void UMythosGameplayAbility::PlayAbilityEffect()
{
    if (AbilityEffect && GetOwningActorFromActorInfo())
    {

    }
}

const UMythosAttributeSet* UMythosGameplayAbility::GetMythosAttributeSet() const
{
    if (UAbilitySystemComponent* LocalASC = GetAbilitySystemComponentFromActorInfo())
    {
        return Cast<UMythosAttributeSet>(LocalASC->GetAttributeSet(UMythosAttributeSet::StaticClass()));
    }
    return nullptr;
}

float UMythosGameplayAbility::GetAbilityRange() const
{
    switch (AbilityType)
    {
    case EMythosAbilityType::Targeted:
    case EMythosAbilityType::Directional:
        return AbilityDistance;
    case EMythosAbilityType::Area:
        return AbilityRadius;
    case EMythosAbilityType::Self:
        return SelfEffectRadius;
    default:
        return 0.0f;
    }
}

bool UMythosGameplayAbility::IsTargetInRange(const FVector& CasterLocation, const FVector& TargetLocation, const FVector& CasterForward) const
{
    const float Distance = FVector::Dist(CasterLocation, TargetLocation);
    
    switch (AbilityType)
    {
    case EMythosAbilityType::Self:
        return Distance <= SelfEffectRadius;
        
    case EMythosAbilityType::Targeted:
        return Distance <= AbilityDistance;
        
    case EMythosAbilityType::Directional:
    {
        if (Distance > AbilityDistance)
            return false;
            
        // Check angle (cone range)
        if (CasterForward.IsNearlyZero())
            return true;
            
        const FVector DirectionToTarget = (TargetLocation - CasterLocation).GetSafeNormal();
        const float DotProduct = FVector::DotProduct(CasterForward, DirectionToTarget);
        const float Angle = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)) * 180.0f / PI;
        
        return Angle <= (AbilityAngle * 0.5f);
    }
        
    case EMythosAbilityType::Area:
        return Distance <= AbilityRadius;
        
    default:
        return false;
    }
}

float UMythosGameplayAbility::GetEffectRadius() const
{
    switch (AbilityType)
    {
    case EMythosAbilityType::Self:
        return SelfEffectRadius;
    case EMythosAbilityType::Area:
        return AbilityRadius;
    case EMythosAbilityType::Directional:
        return AbilityDistance; // For directional skills, return distance as effect range
    case EMythosAbilityType::Targeted:
        return 0.0f; // Targeted skills usually have no area effect
    default:
        return 0.0f;
    }
}

void UMythosGameplayAbility::BPApplyCooldown()
{
    ApplyCooldown(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo());
}

void UMythosGameplayAbility::BPApplyCost()
{
    ApplyCost(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo());
}

bool UMythosGameplayAbility::BPCheckCost()
{
    return CheckCost(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
}

// get all characters in the range of the ability by trace
TArray<AActor*> UMythosGameplayAbility::GetAbilityTargets(FGameplayTag TagFilter)
{
    TArray<AActor*> Result;
    AMythosCharacter* OwnerChar = Cast<AMythosCharacter>(GetAvatarActorFromActorInfo());
    if (!OwnerChar) return Result;

    AMythosPlayerController* PC = Cast<AMythosPlayerController>(OwnerChar->GetController());
    if (!PC) return Result;

    FVector MouseWorldLoc, MouseWorldDir;
    FHitResult Hit;
    PC->GetMouseWorldPosition(MouseWorldLoc, MouseWorldDir, Hit);
    //GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Green, FString::Printf(TEXT("Hit Actor %s"), *Hit.GetActor()->GetName()));


    // Character rotation is now handled in ActivateAbility

    UWorld* World = GetWorld();
    if (!World) return Result;

    // all characters in the range
    TArray<AActor*> AllCharacters;
    UGameplayStatics::GetAllActorsOfClass(World, AMythosCharacter::StaticClass(), AllCharacters);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerChar);

    switch (AbilityType)
    {
        case EMythosAbilityType::Area:
        {
            // mouse pointer is in the range of the ability
            if (FVector::Dist(Hit.Location, OwnerChar->GetActorLocation()) <= AbilityDistance)
            {
                // Sphere trace at mouse location
                TArray<FHitResult> HitResults;
                FVector TraceCenter = Hit.Location;
                float TraceRadius = AbilityRadius;
                DrawDebugSphere(World, TraceCenter, TraceRadius, 32, FColor::Green, false, 2.0f);
                World->SweepMultiByObjectType(
                    HitResults,
                    TraceCenter,
                    TraceCenter,
                    FQuat::Identity,
                    FCollisionObjectQueryParams(ECC_Pawn),
                    FCollisionShape::MakeSphere(TraceRadius),
                    QueryParams
                );
                for (const FHitResult& HR : HitResults)
                {
                    AActor* Actor = HR.GetActor();
                    if (Actor && Actor != OwnerChar)
                    {
                        if (TagFilter.IsValid() && Actor->Implements<UGameplayTagAssetInterface>())
                        {
                            IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Actor);
                            if (TagInterface && TagInterface->HasMatchingGameplayTag(TagFilter))
                                Result.AddUnique(Actor);
                        }
                        else if (!TagFilter.IsValid())
                        {
                            Result.AddUnique(Actor);
                        }
                    }
                }
            }
            break;
        }
        case EMythosAbilityType::Self:
        {
            // Sphere trace at character location
            TArray<FHitResult> HitResults;
            FVector TraceCenter = OwnerChar->GetActorLocation();
            float TraceRadius = AbilityRadius;
            DrawDebugSphere(World, TraceCenter, TraceRadius, 32, FColor::Blue, false, 2.0f);
            World->SweepMultiByObjectType(
                HitResults,
                TraceCenter,
                TraceCenter,
                FQuat::Identity,
                FCollisionObjectQueryParams(ECC_Pawn),
                FCollisionShape::MakeSphere(TraceRadius),
                QueryParams
            );
            for (const FHitResult& HR : HitResults)
            {
                AActor* Actor = HR.GetActor();
                if (Actor && Actor != OwnerChar)
                {
                    if (TagFilter.IsValid() && Actor->Implements<UGameplayTagAssetInterface>())
                    {
                        IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Actor);
                        if (TagInterface && TagInterface->HasMatchingGameplayTag(TagFilter))
                            Result.AddUnique(Actor);
                    }
                    else if (!TagFilter.IsValid())
                    {
                        Result.AddUnique(Actor);
                    }
                }
            }
            break;
        }
        case EMythosAbilityType::Directional:
        {
            // Calculate direction from character to mouse position
            FVector DirectionToMouse = (Hit.Location - OwnerChar->GetActorLocation());
            DirectionToMouse.Z = 0.0f; // Ignore Z axis, only consider horizontal direction
            DirectionToMouse = DirectionToMouse.GetSafeNormal();
            
            // Cone trace from character location
            TArray<FHitResult> HitResults;
            FVector TraceStart = OwnerChar->GetActorLocation() + FVector(0, 0, 30.0f);
            FVector TraceEnd = TraceStart + DirectionToMouse * AbilityDistance;
            float TraceRadius = AbilityRadius;
            
            // Draw debug cone
            DrawDebugCone(World, TraceStart, DirectionToMouse, AbilityDistance, FMath::DegreesToRadians(AbilityAngle * 0.5f), FMath::DegreesToRadians(AbilityAngle * 0.5f), 32, FColor::Red, false, 2.0f);
            
            // Use capsule trace for cone shape
            FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(TraceRadius, AbilityDistance * 0.5f);
            World->SweepMultiByObjectType(
                HitResults,
                TraceStart,
                TraceEnd,
                DirectionToMouse.Rotation().Quaternion(),
                FCollisionObjectQueryParams(ECC_Pawn),
                CapsuleShape,
                QueryParams
            );
            
            for (const FHitResult& HR : HitResults)
            {
                AActor* Actor = HR.GetActor();
                if (Actor && Actor != OwnerChar)
                {
                    if (TagFilter.IsValid() && Actor->Implements<UGameplayTagAssetInterface>())
                    {
                        IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Actor);
                        if (TagInterface && TagInterface->HasMatchingGameplayTag(TagFilter))
                            Result.AddUnique(Actor);
                    }
                    else if (!TagFilter.IsValid())
                    {
                        Result.AddUnique(Actor);
                    }
                }
            }
            break;
        }
        case EMythosAbilityType::Targeted:
        {
            if (AActor* HitActor = Hit.GetActor())
            {
                if (HitActor->IsA(AMythosCharacter::StaticClass()))
                {
                    if (TagFilter.IsValid() && HitActor->Implements<UGameplayTagAssetInterface>())
                    {
                        IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(HitActor);
                        if (TagInterface && TagInterface->HasMatchingGameplayTag(TagFilter))
                            Result.AddUnique(HitActor);
                    }
                    else if (!TagFilter.IsValid())
                    {
                        Result.AddUnique(HitActor);
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    return Result;
}

bool UMythosGameplayAbility::GetMouseDirectionAndPosition(FVector& OutDirection, FVector& OutPosition) const
{
    // Get player controller
    AMythosPlayerController* PC = Cast<AMythosPlayerController>(GetOwningActorFromActorInfo()->GetInstigatorController());
    if (!PC)
    {
        return false;
    }

    // Get mouse world position and direction
    FVector MouseWorldLoc, MouseWorldDir;
    FHitResult Hit;
    if (PC->GetMouseWorldPosition(MouseWorldLoc, MouseWorldDir, Hit))
    {
        // Get the avatar actor (character)
        AMythosCharacter* OwnerChar = Cast<AMythosCharacter>(GetAvatarActorFromActorInfo());
        if (OwnerChar && Hit.bBlockingHit)
        {
            // Use Hit.Location as mouse world position
            OutPosition = Hit.Location;
            
            // Calculate direction from character to mouse position (ignore Z axis)
            FVector DirectionToMouse = (Hit.Location - OwnerChar->GetActorLocation());
            DirectionToMouse.Z = 0.0f; // Ignore Z axis, only consider horizontal direction
            OutDirection = DirectionToMouse.GetSafeNormal();
        }
        else
        {
            // Fallback to original behavior if no character or no hit
            OutPosition = MouseWorldLoc;
            OutDirection = MouseWorldDir;
        }
        return true;
    }

    return false;
}

TArray<AActor*> UMythosGameplayAbility::GetEnemyAbilityTargets(FGameplayTag TagFilter)
{
    TArray<AActor*> Result;
    AMythosCharacter* OwnerChar = Cast<AMythosCharacter>(GetAvatarActorFromActorInfo());
    if (!OwnerChar) return Result;

    UWorld* World = GetWorld();
    if (!World) return Result;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerChar);

    switch (AbilityType)
    {
        case EMythosAbilityType::Self:
        {
            // Self skill - detect around the enemy
            TArray<FHitResult> HitResults;
            FVector TraceCenter = OwnerChar->GetActorLocation();
            float TraceRadius = SelfEffectRadius;
            DrawDebugSphere(World, TraceCenter, TraceRadius, 32, FColor::Blue, false, 2.0f);
            World->SweepMultiByObjectType(
                HitResults,
                TraceCenter,
                TraceCenter,
                FQuat::Identity,
                FCollisionObjectQueryParams(ECC_Pawn),
                FCollisionShape::MakeSphere(TraceRadius),
                QueryParams
            );
            for (const FHitResult& HR : HitResults)
            {
                AActor* Actor = HR.GetActor();
                if (Actor && Actor != OwnerChar)
                {
                    if (TagFilter.IsValid() && Actor->Implements<UGameplayTagAssetInterface>())
                    {
                        IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Actor);
                        if (TagInterface && TagInterface->HasMatchingGameplayTag(TagFilter))
                            Result.AddUnique(Actor);
                    }
                    else if (!TagFilter.IsValid())
                    {
                        Result.AddUnique(Actor);
                    }
                }
            }
            break;
        }
        case EMythosAbilityType::Directional:
        {
            // Directional skill - attack forward
            FVector AttackDirection = OwnerChar->GetActorForwardVector();
            
            // Cone trace from character location
            TArray<FHitResult> HitResults;
            FVector TraceStart = OwnerChar->GetActorLocation() + FVector(0, 0, 30.0f);
            FVector TraceEnd = TraceStart + AttackDirection * AbilityDistance;
            float TraceRadius = AbilityRadius;
            
            // Draw debug cone
            DrawDebugCone(World, TraceStart, AttackDirection, AbilityDistance, FMath::DegreesToRadians(AbilityAngle * 0.5f), FMath::DegreesToRadians(AbilityAngle * 0.5f), 32, FColor::Red, false, 2.0f);
            
            // Use capsule trace for cone shape
            FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(TraceRadius, AbilityDistance * 0.5f);
            World->SweepMultiByObjectType(
                HitResults,
                TraceStart,
                TraceEnd,
                AttackDirection.Rotation().Quaternion(),
                FCollisionObjectQueryParams(ECC_Pawn),
                CapsuleShape,
                QueryParams
            );
            
            for (const FHitResult& HR : HitResults)
            {
                AActor* Actor = HR.GetActor();
                if (Actor && Actor != OwnerChar)
                {
                    if (TagFilter.IsValid() && Actor->Implements<UGameplayTagAssetInterface>())
                    {
                        IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Actor);
                        if (TagInterface && TagInterface->HasMatchingGameplayTag(TagFilter))
                            Result.AddUnique(Actor);
                    }
                    else if (!TagFilter.IsValid())
                    {
                        Result.AddUnique(Actor);
                    }
                }
            }
            break;
        }
        case EMythosAbilityType::Area:
        {
            // AOE skill - detect in front based on skill distance
            FVector AttackDirection = OwnerChar->GetActorForwardVector();
            FVector AoeCenter = OwnerChar->GetActorLocation() + AttackDirection * AbilityDistance;
            
            TArray<FHitResult> HitResults;
            float TraceRadius = AbilityRadius;
            DrawDebugSphere(World, AoeCenter, TraceRadius, 32, FColor::Green, false, 2.0f);
            World->SweepMultiByObjectType(
                HitResults,
                AoeCenter,
                AoeCenter,
                FQuat::Identity,
                FCollisionObjectQueryParams(ECC_Pawn),
                FCollisionShape::MakeSphere(TraceRadius),
                QueryParams
            );
            
            for (const FHitResult& HR : HitResults)
            {
                AActor* Actor = HR.GetActor();
                if (Actor && Actor != OwnerChar)
                {
                    if (TagFilter.IsValid() && Actor->Implements<UGameplayTagAssetInterface>())
                    {
                        IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Actor);
                        if (TagInterface && TagInterface->HasMatchingGameplayTag(TagFilter))
                            Result.AddUnique(Actor);
                    }
                    else if (!TagFilter.IsValid())
                    {
                        Result.AddUnique(Actor);
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    return Result;
}

void UMythosGameplayAbility::AddTagToActorForDuration(AActor* TargetActor, FGameplayTag TagToAdd, float Duration)
{
    if (!TargetActor || !TagToAdd.IsValid() || Duration <= 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("quit add tag to actor for duration"));
        return;
    }

    // get ASC from target actor
    UAbilitySystemComponent* TargetASC = nullptr;
    if (AMythosCharacter* TargetChar = Cast<AMythosCharacter>(TargetActor))
    {
        TargetASC = TargetChar->GetAbilitySystemComponent();
    }
    else
    {
        // get ASC from target actor
        TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();
    }

    if (!TargetASC)
    {
        UE_LOG(LogTemp, Warning, TEXT("AddTagToActorForDuration: Target actor has no AbilitySystemComponent"));
        return;
    }

    // Loose Tag
    TargetASC->AddLooseGameplayTag(TagToAdd);
    

    FGameplayTagContainer CurrentTags;
    TargetASC->GetOwnedGameplayTags(CurrentTags);
/*    UE_LOG(LogTemp, Log, TEXT("Current tags after adding: %s"), *CurrentTags.ToString());
    
    if (CurrentTags.HasTag(TagToAdd))
    {
        UE_LOG(LogTemp, Log, TEXT("Tag verification: SUCCESS - Tag %s is present"), *TagToAdd.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Tag verification: FAILED - Tag %s is NOT present"), *TagToAdd.ToString());
    }
*/    
    // add tag to actor using timer for duration
    FTimerHandle TimerHandle;
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindLambda([TargetASC, TagToAdd]()
    {
        if (TargetASC && TagToAdd.IsValid())
        {
            TargetASC->RemoveLooseGameplayTag(TagToAdd);
            UE_LOG(LogTemp, Log, TEXT("Removed tag %s from %s"), *TagToAdd.ToString(), *TargetASC->GetOwner()->GetName());
        }
    });
    
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, false);
}

bool UMythosGameplayAbility::FaceMousePosition()
{
    // Get the avatar actor (character)
    AMythosCharacter* OwnerChar = Cast<AMythosCharacter>(GetAvatarActorFromActorInfo());
    if (!OwnerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("FaceMousePosition: No valid character found"));
        return false;
    }

    // Get player controller
    AMythosPlayerController* PC = Cast<AMythosPlayerController>(OwnerChar->GetController());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("FaceMousePosition: No valid player controller found"));
        return false;
    }

    // Get mouse world position and direction
    FVector MouseWorldLoc, MouseWorldDir;
    FHitResult Hit;
    if (!PC->GetMouseWorldPosition(MouseWorldLoc, MouseWorldDir, Hit))
    {
        UE_LOG(LogTemp, Warning, TEXT("FaceMousePosition: Failed to get mouse world position"));
        return false;
    }

    // Check if we have a valid hit result
    if (!Hit.bBlockingHit)
    {
        UE_LOG(LogTemp, Warning, TEXT("FaceMousePosition: No blocking hit found"));
        return false;
    }

    // Use Hit.Location as mouse world position
    FVector MouseWorldPosition = Hit.Location;
    
    // Calculate the direction from character to mouse position (ignore Z axis)
    FVector DirectionToMouse = (MouseWorldPosition - OwnerChar->GetActorLocation());
    DirectionToMouse.Z = 0.0f; // Ignore Z axis, only consider horizontal direction
    DirectionToMouse = DirectionToMouse.GetSafeNormal();
    
    UE_LOG(LogTemp, Warning, TEXT("Direction to mouse: %s"), *DirectionToMouse.ToString());

    if (!DirectionToMouse.IsNearlyZero())
    {
        // Set character rotation
        FRotator NewRotation = DirectionToMouse.Rotation();
        
        UE_LOG(LogTemp, Warning, TEXT("Setting character rotation: %s"), *NewRotation.ToString());
        
        // Set character rotation
        OwnerChar->SetActorRotation(NewRotation);
        
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("FaceMousePosition: Direction to mouse is nearly zero"));
    return false;
}

bool UMythosGameplayAbility::FaceMousePositionSmooth(float CustomRotationSpeed)
{
    // Get the avatar actor (character)
    AMythosCharacter* OwnerChar = Cast<AMythosCharacter>(GetOwningActorFromActorInfo());
    if (!OwnerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("FaceMousePositionSmooth: No valid character found"));
        return false;
    }

    // Get player controller
    AMythosPlayerController* PC = Cast<AMythosPlayerController>(OwnerChar->GetController());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("FaceMousePositionSmooth: No valid player controller found"));
        return false;
    }

    // Get mouse world position and direction
    FVector MouseWorldLoc, MouseWorldDir;
    FHitResult Hit;
    if (!PC->GetMouseWorldPosition(MouseWorldLoc, MouseWorldDir, Hit))
    {
        UE_LOG(LogTemp, Warning, TEXT("FaceMousePositionSmooth: Failed to get mouse world position"));
        return false;
    }

    // Check if we have a valid hit result
    if (!Hit.bBlockingHit)
    {
        UE_LOG(LogTemp, Warning, TEXT("FaceMousePositionSmooth: No blocking hit found"));
        return false;
    }

    // Use Hit.Location as mouse world position
    FVector MouseWorldPosition = Hit.Location;
    
    // Calculate the direction from character to mouse position (ignore Z axis)
    FVector DirectionToMouse = (MouseWorldPosition - OwnerChar->GetActorLocation());
    DirectionToMouse.Z = 0.0f; // Ignore Z axis, only consider horizontal direction
    DirectionToMouse = DirectionToMouse.GetSafeNormal();
    
    UE_LOG(LogTemp, Warning, TEXT("Direction to mouse: %s"), *DirectionToMouse.ToString());

    if (!DirectionToMouse.IsNearlyZero())
    {
        // Calculate rotation duration based on rotation speed
        // Higher rotation speed = shorter duration
        float RotationDuration = 1.0f / FMath::Max(CustomRotationSpeed, 1.0f);
        
        // Call the character's smooth rotation method
        OwnerChar->SmoothRotateToDirection(DirectionToMouse, RotationDuration);
        
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("FaceMousePositionSmooth: Direction to mouse is nearly zero"));
    return false;
}

bool UMythosGameplayAbility::StartSmoothRotationToMouse()
{
    // Get the avatar actor (character)
    AMythosCharacter* OwnerChar = Cast<AMythosCharacter>(GetOwningActorFromActorInfo());
    if (!OwnerChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("StartSmoothRotationToMouse: No valid character found"));
        return false;
    }

    // Get player controller
    AMythosPlayerController* PC = Cast<AMythosPlayerController>(OwnerChar->GetController());
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("StartSmoothRotationToMouse: No valid player controller found"));
        return false;
    }

    // Get mouse world position and direction
    FVector MouseWorldLoc, MouseWorldDir;
    FHitResult Hit;
    if (!PC->GetMouseWorldPosition(MouseWorldLoc, MouseWorldDir, Hit))
    {
        UE_LOG(LogTemp, Warning, TEXT("StartSmoothRotationToMouse: Failed to get mouse world position"));
        return false;
    }

    // Check if we have a valid hit result
    if (!Hit.bBlockingHit)
    {
        UE_LOG(LogTemp, Warning, TEXT("StartSmoothRotationToMouse: No blocking hit found"));
        return false;
    }

    // Use Hit.Location as mouse world position
    FVector MouseWorldPosition = Hit.Location;
    
    // Calculate the direction from character to mouse position (ignore Z axis)
    FVector DirectionToMouse = (MouseWorldPosition - OwnerChar->GetActorLocation());
    DirectionToMouse.Z = 0.0f; // Ignore Z axis, only consider horizontal direction
    DirectionToMouse = DirectionToMouse.GetSafeNormal();
    
    UE_LOG(LogTemp, Warning, TEXT("Direction to mouse: %s"), *DirectionToMouse.ToString());

    if (!DirectionToMouse.IsNearlyZero())
    {
        // Calculate rotation duration based on the ability's rotation speed
        // Higher rotation speed = shorter duration
        float RotationDuration = 1.0f / FMath::Max(RotationSpeed, 1.0f);
        
        // Call the character's smooth rotation method
        OwnerChar->SmoothRotateToDirection(DirectionToMouse, RotationDuration);
        
        UE_LOG(LogTemp, Warning, TEXT("Started smooth rotation with speed: %.2f, duration: %.2f"), RotationSpeed, RotationDuration);
        
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("StartSmoothRotationToMouse: Direction to mouse is nearly zero"));
    return false;
}