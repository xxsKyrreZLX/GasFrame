// Copyright Epic Games, Inc. All Rights Reserved.

#include "MythosEnemyBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/AbilitySystem/Component/MythosAbilitySystemComponent.h"

void AMythosEnemyBase::UpdateMaxWalkSpeed(float NewMaxWalkSpeed)
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = NewMaxWalkSpeed;
	}
}

void AMythosEnemyBase::InitializeCharacterTypeTags()
{
	if (AbilitySystemComponent)
	{
		// Add Enemy tag instead of Player tag
		FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(TEXT("CharacterType.Enemy"));
		AbilitySystemComponent->AddLooseGameplayTag(EnemyTag);
	}
}

