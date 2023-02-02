// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMovementComponent.h"

#include "GameFramework/GameStateBase.h"

// Sets default values for this component's properties
UGoKartMovementComponent::UGoKartMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UGoKartMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}

bool UGoKartMovementComponent::IsLocallyControlled()
{
	auto* Owner = Cast<APawn>(GetOwner());
	if (!Owner)
	{
		return false;
	}

	return Owner->IsLocallyControlled();
}

// Called every frame
void UGoKartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() == ROLE_AutonomousProxy || IsLocallyControlled())
	{
		FGoKartMove NewMove;
		NewMove.DeltaTime = DeltaTime;
		NewMove.Time = GetOwner()->GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
		NewMove.SteeringThrow = SteeringThrow;
		NewMove.Throttle = Throttle;

		LastMove = NewMove;

		SimulateMove(NewMove);
	}
}

FGoKartMove UGoKartMovementComponent::GetMove()
{
	return LastMove;
}

FVector UGoKartMovementComponent::GetVelocity()
{
	return Velocity;
}

void UGoKartMovementComponent::SetVelocity(FVector VelocityToSet)
{
	Velocity = VelocityToSet;
}

void UGoKartMovementComponent::SetThrottle(float ThrottleToSet)
{
	Throttle = ThrottleToSet;
}

void UGoKartMovementComponent::SetSteeringThrow(float SteeringThrowToSet)
{
	SteeringThrow = SteeringThrowToSet;
}

void UGoKartMovementComponent::SimulateMove(const FGoKartMove& MoveToSimulate)
{
	FVector Force = GetOwner()->GetActorForwardVector() * MaxDrivingForce * MoveToSimulate.Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	Velocity += Acceleration * MoveToSimulate.DeltaTime;

	ApplyRotation(MoveToSimulate.DeltaTime, MoveToSimulate.SteeringThrow);

	UpdateLocationFromVelocity(MoveToSimulate.DeltaTime);
}

void UGoKartMovementComponent::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;

	FHitResult HitResult;
	GetOwner()->AddActorWorldOffset(Translation, true, &HitResult);
	if (HitResult.IsValidBlockingHit())
	{
		// We have hit a wall, reset velocity
		Velocity = FVector(0,0,0);
	}
}

void UGoKartMovementComponent::ApplyRotation(float DeltaTime, float SteeringThrowToSet)
{
	// Turning Circle: dx = dTheta * r
	float SteeringAngle = (FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime) / MinTurningRadius;

	// Compute the rotation angle in radians
	float RotationAngle = SteeringAngle * SteeringThrowToSet;

	FQuat RotationDelta(GetOwner()->GetActorUpVector(), RotationAngle);
	GetOwner()->AddActorWorldRotation(RotationDelta);

	Velocity = RotationDelta.RotateVector(Velocity);
}

FVector UGoKartMovementComponent::GetAirResistance()
{
	return - Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector UGoKartMovementComponent::GetRollingResistance()
{
	float AccelerationDueToGravity = - GetWorld()->GetGravityZ()/100;
	float NormalForce = Mass * AccelerationDueToGravity;

	return - Velocity.GetSafeNormal() * NormalForce * RRCoefficient;
}
