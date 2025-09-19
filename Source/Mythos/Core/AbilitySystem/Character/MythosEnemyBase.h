// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MythosCharacter.h"
#include "GameplayTags.h"
#include "MythosEnemyBase.generated.h"

/**
 * Base class for enemy characters in the Mythos game
 */
UCLASS()
class MYTHOS_API AMythosEnemyBase : public AMythosCharacter
{
	GENERATED_BODY()
	
public:
	// Update the character movement's max walk speed
	UFUNCTION(BlueprintCallable, Category = "Mythos|Enemy|Movement")
	void UpdateMaxWalkSpeed(float NewMaxWalkSpeed);

protected:
	// Override to add Enemy tag instead of Player tag
	virtual void InitializeCharacterTypeTags() override;
};
