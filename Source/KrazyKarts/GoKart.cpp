// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"

#include "Components/InputComponent.h"

#include "Engine/World.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();

}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;

	FHitResult HitResult;
	AddActorWorldOffset(Translation, true, &HitResult);
	if (HitResult.IsValidBlockingHit())
	{
		// We have hit a wall, reset velocity
		Velocity = FVector(0,0,0);
	}
}

void AGoKart::ApplyRotation(float DeltaTime)
{
	// Turning Circle: dx = dTheta * r
	float SteeringAngle = (FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime) / MinTurningRadius;
	UE_LOG(LogTemp, Display, TEXT("Steering Angle: %f"), SteeringAngle);

	// Compute the rotation angle in radians
	float RotationAngle = SteeringAngle * SteeringThrow;

	UE_LOG(LogTemp, Display, TEXT("Rotation Angle: %f"), RotationAngle);
	FQuat RotationDelta(GetActorUpVector(), RotationAngle);
	AddActorWorldRotation(RotationDelta);

	Velocity = RotationDelta.RotateVector(Velocity);
}

FVector AGoKart::GetAirResistance()
{
	return - Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector AGoKart::GetRollingResistance()
{
	float AccelerationDueToGravity = - GetWorld()->GetGravityZ()/100;
	float NormalForce = Mass * AccelerationDueToGravity;

	return - Velocity.GetSafeNormal() * NormalForce * RRCoefficient;
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	Velocity += Acceleration * DeltaTime;

	ApplyRotation(DeltaTime);

	UpdateLocationFromVelocity(DeltaTime);
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);

	UE_LOG(LogTemp, Display, TEXT("Setup player input"));
}

void AGoKart::MoveForward(float Value)
{
	Throttle = Value;
}

void AGoKart::MoveRight(float Value)
{
	SteeringThrow = Value;
}

