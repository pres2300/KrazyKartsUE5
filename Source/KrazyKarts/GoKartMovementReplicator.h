// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GoKartMovementComponent.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementReplicator.generated.h"

USTRUCT()
struct FGoKartState
{
    GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FGoKartMove LastMove;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FTransform Transform;
};

struct FHermiteCubicSpline
{
	FVector StartLocation, StartDerivative, TargetLocation, TargetDerivative;

	FVector InterpolateLocation(float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}

	FVector InterpolateDerivative(float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementReplicator : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGoKartMovementReplicator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove NewMove);

private:
	TArray<FGoKartMove> UnacknowledgedMoves;

	UPROPERTY(ReplicatedUsing=OnRep_ServerState)
	FGoKartState ServerState;

	UPROPERTY()
	UGoKartMovementComponent* MovementComponent;

	float ClientSimulatedTime;

	float ClientTimeSinceUpdate;

	float ClientTimeBetweenLastUpdates;

	FTransform ClientStartTransform;

	FVector ClientStartVelocity;

	UPROPERTY()
	USceneComponent* MeshOffsetRoot;

	UFUNCTION(BlueprintCallable)
	void SetMeshOffsetRoot(USceneComponent* Root){ MeshOffsetRoot = Root; };

	UFUNCTION()
	void OnRep_ServerState();

	void SimulatedProxy_OnRep_ServerState();

	void AutonomousProxy_OnRep_ServerState();

	void ClearAcknowledgedMoves(float LastMoveTime);

	void UpdateServerState(const FGoKartMove& Move);

	bool IsLocallyControlled();

	void ClientTick(float DeltaTime);

	FHermiteCubicSpline CreateSpline();

	void InterpolateLocation(const FHermiteCubicSpline &Spline, float LerpRatio);

	void InterpolateVelocity(const FHermiteCubicSpline &Spline, float LerpRatio);

	void InterpolateRotation(float LerpRatio);

	float VelocityToDerivative();
};
