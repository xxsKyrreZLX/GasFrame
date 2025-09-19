// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MythosPlayerController.generated.h"

class UInputMappingContext;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class AMythosPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Input", meta = (AllowPrivateAccess = "true"))
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	virtual void BeginPlay() override;

public:
	/** mouse world position */
	UFUNCTION(BlueprintCallable, Category = "Mythos|Mouse")

/*+-----------------------------+
| GetMouseWorldPosition       |
|-----------------------------|
| World Location (out)        |
| World Direction (out)       |
| Hit Result (out)            |
| Return Value (bool, out)    |
+-----------------------------+*/

	bool GetMouseWorldPosition(FVector& WorldLocation, FVector& WorldDirection, FHitResult& HitResult) const;

};
