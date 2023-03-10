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

	bool IsValid() const
	{
		return FMath::Abs(Throttle) <= 1.0 && FMath::Abs(SteeringThrow) <= 1.0;
	}
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

	void SetThrottle(float ThrottleToSet);

	void SetSteeringThrow(float SteeringThrowToSet);

	FVector GetVelocity();

	void SetVelocity(FVector VelocityToSet);

	void SimulateMove(const FGoKartMove& MoveToSimulate);

	bool IsLocallyControlled();

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

	FGoKartMove LastMove;

	float Throttle;
	float SteeringThrow;

	void UpdateLocationFromVelocity(float DeltaTime);

	void ApplyRotation(float DeltaTime, float SteeringThrowToSet);

	FVector GetAirResistance();

	FVector GetRollingResistance();
};
