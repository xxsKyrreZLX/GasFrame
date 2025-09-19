// Copyright Epic Games, Inc. All Rights Reserved.

#include "MythosCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Core/AbilitySystem/Component/MythosAbilitySystemComponent.h"
#include "Core/AbilitySystem/Component/MythosAttributeSet.h"
#include "GameplayTagAssetInterface.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AMythosCharacter::AMythosCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// GASAbilitySystemComponent
	AbilitySystemComponent = CreateDefaultSubobject<UMythosAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// GASAttributeSet
	AttributeSet = CreateDefaultSubobject<UMythosAttributeSet>(TEXT("AttributeSet"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AMythosCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// bind attribute change delegate
		if (AttributeSet)
		{
			AttributeSet->OnHealthChanged.AddDynamic(this, &AMythosCharacter::HandleHealthChanged);
			AttributeSet->OnManaChanged.AddDynamic(this, &AMythosCharacter::HandleManaChanged);
			AttributeSet->OnStaminaChanged.AddDynamic(this, &AMythosCharacter::HandleStaminaChanged);
			
			// 绑定GE应用委托
			AttributeSet->OnGameplayEffectApplied.AddDynamic(this, &AMythosCharacter::HandleGameplayEffectApplied);
		}

		// Initialize character type tags
		InitializeCharacterTypeTags();
	}
}

void AMythosCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMythosCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMythosCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMythosCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMythosCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AMythosCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AMythosCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AMythosCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMythosCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AMythosCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

// IGameplayTagAssetInterface implementation
void AMythosCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetOwnedGameplayTags(TagContainer);
	}
}

bool AMythosCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasMatchingGameplayTag(TagToCheck);
	}
	return false;
}

bool AMythosCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasAnyMatchingGameplayTags(TagContainer);
	}
	return false;
}

bool AMythosCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasAllMatchingGameplayTags(TagContainer);
	}
	return false;
}

// attribute change handler
void AMythosCharacter::HandleHealthChanged(float OldHealth, float NewHealth, const FGameplayAttribute& Attribute)
{
	// get max health
	float MaxHealth = 100.0f; // default value, can be get from AttributeSet
	if (AttributeSet)
	{
		MaxHealth = AttributeSet->GetMaxHealth();
	}
	
	// trigger delegate
	OnHealthChanged.Broadcast(OldHealth, NewHealth, MaxHealth);
}

void AMythosCharacter::HandleManaChanged(float OldMana, float NewMana, const FGameplayAttribute& Attribute)
{
	// get max mana
	float MaxMana = 50.0f; // default value, can be get from AttributeSet
	if (AttributeSet)
	{
		MaxMana = AttributeSet->GetMaxMana();
	}
	
	// trigger delegate
	OnManaChanged.Broadcast(OldMana, NewMana, MaxMana);
}

void AMythosCharacter::HandleStaminaChanged(float OldStamina, float NewStamina, const FGameplayAttribute& Attribute)
{
	// get max stamina
	float MaxStamina = 100.0f; // default value, can be get from AttributeSet
	if (AttributeSet)
	{
		MaxStamina = AttributeSet->GetMaxStamina();
	}
	
	// trigger delegate
	OnStaminaChanged.Broadcast(OldStamina, NewStamina, MaxStamina);
}

void AMythosCharacter::HandleGameplayEffectApplied(AActor* Source, FString EffectName, float Magnitude)
{
	/* 
	if (GEngine)
	{
		FString DebugMessage = FString::Printf(TEXT("GE Applied to %s: Source=%s, Effect=%s, Magnitude=%.2f"), 
			*GetName(),
			Source ? *Source->GetName() : TEXT("None"),
			*EffectName,
			Magnitude);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, DebugMessage);
	}
*/
	// call delegate in blueprint
	OnGameplayEffectApplied.Broadcast(Source, EffectName, Magnitude);
}

void AMythosCharacter::InitializeCharacterTypeTags()
{
	if (AbilitySystemComponent)
	{
		// Add Player tag by default
		FGameplayTag PlayerTag = FGameplayTag::RequestGameplayTag(TEXT("CharacterType.Player"));
		AbilitySystemComponent->AddLooseGameplayTag(PlayerTag);
	}
}

void AMythosCharacter::SmoothRotateToDirection(const FVector& TargetDirection, float Duration)
{
	// Stop any existing smooth rotation
	StopSmoothRotation();

	// Normalize the target direction
	FVector NormalizedDirection = TargetDirection.GetSafeNormal();
	if (NormalizedDirection.IsNearlyZero())
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("SmoothRotateToDirection: Target direction is nearly zero"));
		return;
	}

	// Calculate target rotation from direction
	FRotator TargetRotation = NormalizedDirection.Rotation();
	
	// Set up smooth rotation
	SmoothRotationStart = GetActorRotation();
	SmoothRotationTarget = TargetRotation;
	SmoothRotationDuration = FMath::Max(Duration, 0.1f); // Minimum duration of 0.1 seconds
	SmoothRotationElapsed = 0.0f;

	UE_LOG(LogTemplateCharacter, Log, TEXT("Starting smooth rotation: Start=%s, Target=%s, Duration=%.2f"), 
		*SmoothRotationStart.ToString(), *SmoothRotationTarget.ToString(), SmoothRotationDuration);

	// Start the timer for smooth rotation updates
	GetWorld()->GetTimerManager().SetTimer(SmoothRotationTimerHandle, this, &AMythosCharacter::UpdateSmoothRotation, 
		GetWorld()->GetDeltaSeconds(), true);
}

void AMythosCharacter::StopSmoothRotation()
{
	if (SmoothRotationTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(SmoothRotationTimerHandle);
		UE_LOG(LogTemplateCharacter, Log, TEXT("Stopped smooth rotation"));
	}
}

void AMythosCharacter::UpdateSmoothRotation()
{
	// Update elapsed time
	SmoothRotationElapsed += GetWorld()->GetDeltaSeconds();
	
	// Calculate interpolation alpha (0.0 to 1.0)
	float Alpha = FMath::Clamp(SmoothRotationElapsed / SmoothRotationDuration, 0.0f, 1.0f);
	
	// Use smooth interpolation (ease in/out)
	float SmoothAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);
	
	// Interpolate rotation
	FRotator NewRotation = FMath::Lerp(SmoothRotationStart, SmoothRotationTarget, SmoothAlpha);
	
	// Apply the rotation
	SetActorRotation(NewRotation);
	
	UE_LOG(LogTemplateCharacter, Log, TEXT("Smooth rotation update: Alpha=%.2f, Rotation=%s"), Alpha, *NewRotation.ToString());
	
	// Check if rotation is complete
	if (Alpha >= 1.0f)
	{
		// Set final rotation exactly
		SetActorRotation(SmoothRotationTarget);
		StopSmoothRotation();
		UE_LOG(LogTemplateCharacter, Log, TEXT("Smooth rotation completed"));
	}
}
