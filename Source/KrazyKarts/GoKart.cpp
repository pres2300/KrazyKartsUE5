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

	MovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>(TEXT("MovementComponent"));
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

	if (MovementComponent == nullptr)
	{
		return;
	}

	FGoKartMove Move = MovementComponent->GetMove();

	Move.DeltaTime = DeltaTime;
	Move.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Add(Move);
		MovementComponent->SimulateMove(Move);

		Server_SendMove(Move);
	}

	// We are the server and in control of the pawn
	if (GetLocalRole() == ROLE_Authority && IsLocallyControlled())
	{
		Server_SendMove(Move);
	}

	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove);
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
	if (MovementComponent == nullptr)
	{
		return;
	}

	SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	ClearAcknowledgedMoves(ServerState.LastMove.Time);

	for (int32 i=0; i<UnacknowledgedMoves.Num(); i++)
	{
		MovementComponent->SimulateMove(UnacknowledgedMoves[i]);
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
	if (MovementComponent == nullptr)
	{
		return;
	}

	MovementComponent->SetThrottle(Value);
}

void AGoKart::MoveRight(float Value)
{
	if (MovementComponent == nullptr)
	{
		return;
	}

	MovementComponent->SetSteeringThrow(Value);
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove NewMove)
{
	if (MovementComponent == nullptr)
	{
		return;
	}

	MovementComponent->SimulateMove(NewMove);

	ServerState.LastMove = NewMove;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
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
