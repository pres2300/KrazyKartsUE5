// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"

#include "Components/InputComponent.h"

#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AGoKart::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGoKart, ServerState);
	DOREPLIFETIME(AGoKart, Move);
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
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

	// Compute the rotation angle in radians
	float RotationAngle = SteeringAngle * Move.SteeringThrow;

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

FString GetEnumText(ENetRole Role)
{
	switch (Role)
	{
		case ROLE_None:
			return "None";
		case ROLE_SimulatedProxy:
			return "SimulatedProxy";
		case ROLE_AutonomousProxy:
			return "AutonomousProxy";
		case ROLE_Authority:
			return "Authority";
		default:
			return "Error";
	}
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Move.DeltaTime = DeltaTime;
	// TODO: set time

	FVector Force = GetActorForwardVector() * MaxDrivingForce * Move.Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	Velocity += Acceleration * DeltaTime;

	ApplyRotation(DeltaTime);

	UpdateLocationFromVelocity(DeltaTime);

	if (HasAuthority())
	{
		ServerState.Transform = GetActorTransform();
		ServerState.Velocity = Velocity;
		// TODO: update last move
	}
	else if (IsLocallyControlled())
	{
		Server_SendMove(Move);
	}

	DrawDebugString(GetWorld(), FVector(0,0,100), GetEnumText(GetLocalRole()), this, FColor::White, DeltaTime);
}

void AGoKart::OnRep_ServerState()
{
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;
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
	Move.Throttle = Value;
}

void AGoKart::MoveRight(float Value)
{
	Move.SteeringThrow = Value;
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove NewMove)
{
	Move = NewMove;
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove NewMove)
{
	if (FMath::Abs(NewMove.Throttle) > 1.0)
	{
		return false;
	}

	if (FMath::Abs(NewMove.SteeringThrow) > 1.0)
	{
		return false;
	}

	return true;
}
