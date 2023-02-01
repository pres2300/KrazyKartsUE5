// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMovementReplicator.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UGoKartMovementReplicator::UGoKartMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}


// Called when the game starts
void UGoKartMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();
}


// Called every frame
void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UGoKartMovementReplicator::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UGoKartMovementReplicator, ServerState);
}

void UGoKartMovementReplicator::AddUnacknowledgedMove(FGoKartMove NewMove)
{
	UnacknowledgedMoves.Add(NewMove);
}

FGoKartMove UGoKartMovementReplicator::GetLastMove()
{
	return ServerState.LastMove;
}

void UGoKartMovementReplicator::ClearAcknowledgedMoves(float LastMoveTime)
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

void UGoKartMovementReplicator::OnRep_ServerState()
{
	if (MovementComponent == nullptr)
	{
		return;
	}

	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	ClearAcknowledgedMoves(ServerState.LastMove.Time);

	for (int32 i=0; i<UnacknowledgedMoves.Num(); i++)
	{
		MovementComponent->SimulateMove(UnacknowledgedMoves[i]);
	}
}

void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoKartMove NewMove)
{
	if (MovementComponent == nullptr)
	{
		return;
	}

	MovementComponent->SimulateMove(NewMove);

	ServerState.LastMove = NewMove;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove NewMove)
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