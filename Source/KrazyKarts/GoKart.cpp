// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"

#include "Components/InputComponent.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
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

void AGoKart::SimulateMove(const FGoKartMove& MoveToSimulate)
{
	FVector Force = GetActorForwardVector() * MaxDrivingForce * MoveToSimulate.Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	Velocity += Acceleration * MoveToSimulate.DeltaTime;

	ApplyRotation(MoveToSimulate.DeltaTime, MoveToSimulate.SteeringThrow);

	UpdateLocationFromVelocity(MoveToSimulate.DeltaTime);

	Move = MoveToSimulate;
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

void AGoKart::ApplyRotation(float DeltaTime, float SteeringThrow)
{
	// Turning Circle: dx = dTheta * r
	float SteeringAngle = (FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime) / MinTurningRadius;

	// Compute the rotation angle in radians
	float RotationAngle = SteeringAngle * SteeringThrow;

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
	Move.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	if (IsLocallyControlled())
	{
		if (!HasAuthority())
		{
			UnacknowledgedMoves.Add(Move);

			UE_LOG(LogTemp, Warning, TEXT("Queue length: %d"), UnacknowledgedMoves.Num());
		}

		Server_SendMove(Move);

		SimulateMove(Move);
	}

	DrawDebugString(GetWorld(), FVector(0,0,100), GetEnumText(GetLocalRole()), this, FColor::White, DeltaTime);
}

void AGoKart::ClearAcknowledgedMoves(float LastMoveTime)
{
	TArray<FGoKartMove> NewMoves;

	for (const FGoKartMove& MoveItr : UnacknowledgedMoves)
	{
		if (MoveItr.Time > LastMoveTime)
		{
			NewMoves.Add(MoveItr);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

void AGoKart::OnRep_ServerState()
{
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;

	ClearAcknowledgedMoves(ServerState.LastMove.Time);

	for (int32 i=0; i<UnacknowledgedMoves.Num(); i++)
	{
		SimulateMove(UnacknowledgedMoves[i]);
	}
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
	SimulateMove(NewMove);

	ServerState.LastMove = NewMove;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
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
