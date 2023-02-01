// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"

USTRUCT()
struct FGoKartMove
{
    GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGoKartMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FGoKartMove GetMove();

	void SetMove(FGoKartMove MoveToSet);

	void SetThrottle(float ThrottleToSet);

	void SetSteeringThrow(float SteeringThrowToSet);

	FVector GetVelocity();

	void SetVelocity(FVector VelocityToSet);

	void SimulateMove(const FGoKartMove& MoveToSimulate);

private:
	FVector Velocity;

	// The mass of the car (kg)
	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10;	// meters

	// Rolling resistance
	UPROPERTY(EditAnywhere)
	float RRCoefficient =  0.0150; // https://en.wikipedia.org/wiki/Rolling_resistance

	FGoKartMove Move;

	void UpdateLocationFromVelocity(float DeltaTime);

	void ApplyRotation(float DeltaTime, float SteeringThrow);

	FVector GetAirResistance();

	FVector GetRollingResistance();
};
