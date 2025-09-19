// Copyright Epic Games, Inc. All Rights Reserved.


#include "MythosPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"

void AMythosPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Contexts
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}

bool AMythosPlayerController::GetMouseWorldPosition(FVector& WorldLocation, FVector& WorldDirection, FHitResult& HitResult) const
{
	if (!IsLocalController()) return false;
	// get mouse world position and direction
	if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
	{
		FVector Start = WorldLocation;
		FVector End = Start + WorldDirection * 10000.f;
		// do a line trace to get the mouse pointer's ground point
		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
		{
			return true;
		}
	}
	return false;
}

void AMythosPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Always show mouse cursor
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	// Set input mode to Game and UI
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}
