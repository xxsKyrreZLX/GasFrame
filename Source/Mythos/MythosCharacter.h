// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Core/AbilitySystem/Component/MythosAbilitySystemComponent.h"
#include "Core/AbilitySystem/Component/MythosAttributeSet.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTags.h"
#include "MythosCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AMythosCharacter : public ACharacter, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* MouseLookAction;

	// GAS Ability System Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	UMythosAbilitySystemComponent* AbilitySystemComponent;

	// GAS Attribute Set
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	UMythosAttributeSet* AttributeSet;

	// Initialize character type tags - can be overridden by derived classes
	virtual void InitializeCharacterTypeTags();

public:

	/** Constructor */
	AMythosCharacter();	

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** PostInitializeComponents */
	virtual void PostInitializeComponents() override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// GAS
	FORCEINLINE UMythosAbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystemComponent; }
	FORCEINLINE UMythosAttributeSet* GetAttributeSet() const { return AttributeSet; }

	// 获取AttributeSet的蓝图可调用版本
	UFUNCTION(BlueprintCallable, Category = "Mythos|Character|GAS")
	UMythosAttributeSet* GetMythosAttributeSet() const { return AttributeSet; }

	// IGameplayTagAssetInterface implementation
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;

	// delegate
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChangedDelegate, float, OldHealth, float, NewHealth, float, MaxHealth);
	UPROPERTY(BlueprintAssignable, Category = "Mythos|Character|Attributes")
	FOnHealthChangedDelegate OnHealthChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnManaChangedDelegate, float, OldMana, float, NewMana, float, MaxMana);
	UPROPERTY(BlueprintAssignable, Category = "Mythos|Character|Attributes")
	FOnManaChangedDelegate OnManaChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStaminaChangedDelegate, float, OldStamina, float, NewStamina, float, MaxStamina);
	UPROPERTY(BlueprintAssignable, Category = "Mythos|Character|Attributes")
	FOnStaminaChangedDelegate OnStaminaChanged;

	// GE applied delegate
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGameplayEffectAppliedDelegate, AActor*, Source, FString, EffectName, float, Magnitude);
	UPROPERTY(BlueprintAssignable, Category = "Mythos|Character|GAS")
	FOnGameplayEffectAppliedDelegate OnGameplayEffectApplied;

	// Smooth rotation to target direction
	UFUNCTION(BlueprintCallable, Category = "Mythos|Character|Movement")
	void SmoothRotateToDirection(const FVector& TargetDirection, float Duration);

	// Stop smooth rotation
	UFUNCTION(BlueprintCallable, Category = "Mythos|Character|Movement")
	void StopSmoothRotation();

protected:
	// attribute change handler
	UFUNCTION()
	void HandleHealthChanged(float OldHealth, float NewHealth, const FGameplayAttribute& Attribute);

	UFUNCTION()
	void HandleManaChanged(float OldMana, float NewMana, const FGameplayAttribute& Attribute);

	UFUNCTION()
	void HandleStaminaChanged(float OldStamina, float NewStamina, const FGameplayAttribute& Attribute);

	// GE applied handler
	UFUNCTION()
	void HandleGameplayEffectApplied(AActor* Source, FString EffectName, float Magnitude);

private:
	// Smooth rotation variables
	FTimerHandle SmoothRotationTimerHandle;
	FRotator SmoothRotationStart;
	FRotator SmoothRotationTarget;
	float SmoothRotationDuration;
	float SmoothRotationElapsed;

	// Smooth rotation update function
	void UpdateSmoothRotation();
};

