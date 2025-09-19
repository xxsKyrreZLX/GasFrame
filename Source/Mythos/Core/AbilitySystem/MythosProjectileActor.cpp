// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/AbilitySystem/MythosProjectileActor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Core/AbilitySystem/Component/MythosAbilitySystemComponent.h"
#include "MythosCharacter.h"

// Sets default values
AMythosProjectileActor::AMythosProjectileActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create collision component
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetSphereRadius(10.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Create movement component
	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
	MovementComponent->SetUpdatedComponent(CollisionComponent);
	MovementComponent->InitialSpeed = 2000.0f;
	MovementComponent->MaxSpeed = 2000.0f;
	MovementComponent->bRotationFollowsVelocity = true;
	MovementComponent->bShouldBounce = false;
	MovementComponent->ProjectileGravityScale = 0.0f;

	// Collision event binding will be done in Blueprint

	// Initialize data
	Effect = nullptr;
	LifeTime = 5.0f;
	RemainingLifeTime = LifeTime;
	bIsAlive = true;
	MovementDirection = FVector::ForwardVector;
	MovementSpeed = 2000.0f;
}

// Called when the game starts or when spawned
void AMythosProjectileActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMythosProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update life time
	if (bIsAlive)
	{
		RemainingLifeTime -= DeltaTime;
		if (RemainingLifeTime <= 0.0f)
		{
			DestroyProjectile();
		}
	}
}

void AMythosProjectileActor::InitializeProjectile(AActor* InOwner, UGameplayEffect* InEffect, float InLifeTime)
{
	SetOwner(InOwner);
	Effect = InEffect;
	LifeTime = InLifeTime;
	RemainingLifeTime = LifeTime;
	bIsAlive = true;
}

void AMythosProjectileActor::FireProjectile(const FVector& Direction, float Speed)
{
	SetMovementDirection(Direction, Speed);
	
	if (MovementComponent)
	{
		MovementComponent->Velocity = MovementDirection * MovementSpeed;
		MovementComponent->Activate();
	}
}

void AMythosProjectileActor::SetMovementDirection(const FVector& Direction, float Speed)
{
	MovementDirection = Direction.GetSafeNormal();
	MovementSpeed = Speed;
	
	if (MovementComponent)
	{
		MovementComponent->InitialSpeed = Speed;
		MovementComponent->MaxSpeed = Speed;
	}
}

FVector AMythosProjectileActor::GetMovementDirection() const
{
	return MovementDirection;
}

void AMythosProjectileActor::DestroyProjectile()
{
	bIsAlive = false;
	Destroy();
}

AMythosProjectileActor* AMythosProjectileActor::SpawnProjectileWithDirection(
	const UObject* WorldContextObject,
	TSubclassOf<AMythosProjectileActor> ProjectileClass,
	const FVector& SpawnLocation,
	const FVector& Direction,
	AActor* InOwner,
	UGameplayEffect* Effect,
	float LifeTime,
	float Speed)
{
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnProjectileWithDirection: Invalid ProjectileClass"));
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnProjectileWithDirection: Invalid World"));
		return nullptr;
	}

	// Calculate spawn rotation from direction
	FRotator SpawnRotation = Direction.GetSafeNormal().Rotation();

	// Spawn projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = InOwner;
	SpawnParams.Instigator = Cast<APawn>(InOwner);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMythosProjectileActor* Projectile = World->SpawnActor<AMythosProjectileActor>(
		ProjectileClass, 
		SpawnLocation, 
		SpawnRotation, 
		SpawnParams
	);

	if (Projectile)
	{
		// Initialize projectile
		Projectile->InitializeProjectile(InOwner, Effect, LifeTime);
		
		// Set movement direction and fire
		Projectile->SetMovementDirection(Direction, Speed);
		Projectile->FireProjectile(Direction, Speed);
	}

	return Projectile;
}

