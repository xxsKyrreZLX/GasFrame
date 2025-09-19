// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "MythosProjectileActor.generated.h"

class UGameplayEffect;

UCLASS()
class MYTHOS_API AMythosProjectileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMythosProjectileActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Initialize projectile with basic data
	UFUNCTION(BlueprintCallable, Category = "Mythos|Projectile")
	void InitializeProjectile(AActor* InOwner, UGameplayEffect* InEffect, float InLifeTime);

	// Fire the projectile
	UFUNCTION(BlueprintCallable, Category = "Mythos|Projectile")
	void FireProjectile(const FVector& Direction, float Speed);

	// Set movement direction
	UFUNCTION(BlueprintCallable, Category = "Mythos|Projectile")
	void SetMovementDirection(const FVector& Direction, float Speed);

	// Get current movement direction
	UFUNCTION(BlueprintCallable, Category = "Mythos|Projectile")
	FVector GetMovementDirection() const;

	// Check if projectile is alive
	UFUNCTION(BlueprintCallable, Category = "Mythos|Projectile")
	bool IsAlive() const { return bIsAlive; }

	// Get remaining life time
	UFUNCTION(BlueprintCallable, Category = "Mythos|Projectile")
	float GetRemainingLifeTime() const { return RemainingLifeTime; }

	// Destroy projectile
	UFUNCTION(BlueprintCallable, Category = "Mythos|Projectile")
	void DestroyProjectile();

public:
	// Static function to spawn projectile with direction
	UFUNCTION(BlueprintCallable, Category = "Mythos|Projectile", meta = (WorldContext = "WorldContextObject"))
	static AMythosProjectileActor* SpawnProjectileWithDirection(
		const UObject* WorldContextObject,
		TSubclassOf<AMythosProjectileActor> ProjectileClass,
		const FVector& SpawnLocation,
		const FVector& Direction,
		AActor* InOwner,
		UGameplayEffect* Effect,
		float LifeTime = 5.0f,
		float Speed = 2000.0f
	);

protected:
	// Collision component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mythos|Projectile")
	USphereComponent* CollisionComponent;

	// Movement component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mythos|Projectile")
	UProjectileMovementComponent* MovementComponent;

	// Projectile data
	UPROPERTY(BlueprintReadOnly, Category = "Mythos|Projectile")
	UGameplayEffect* Effect;

	UPROPERTY(BlueprintReadOnly, Category = "Mythos|Projectile")
	float LifeTime;

	UPROPERTY(BlueprintReadOnly, Category = "Mythos|Projectile")
	float RemainingLifeTime;

	UPROPERTY(BlueprintReadOnly, Category = "Mythos|Projectile")
	bool bIsAlive;

	// Movement data
	UPROPERTY(BlueprintReadOnly, Category = "Mythos|Projectile")
	FVector MovementDirection;

	UPROPERTY(BlueprintReadOnly, Category = "Mythos|Projectile")
	float MovementSpeed;

};
