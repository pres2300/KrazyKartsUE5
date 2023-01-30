// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
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

	void UpdateLocationFromVelocity(float DeltaTime);

	void ApplyRotation(float DeltaTime);

	FVector GetAirResistance();

	FVector GetRollingResistance();

	void MoveForward(float Value);

	void MoveRight(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveForward(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveRight(float Value);

	UPROPERTY(Replicated)
	FVector Velocity;

	UPROPERTY(ReplicatedUsing=OnRep_ReplicatedTransform)
	FTransform ReplicatedTransform;

	UFUNCTION()
	void OnRep_ReplicatedTransform();

	UPROPERTY(Replicated)
	float Throttle;

	UPROPERTY(Replicated)
	float SteeringThrow;
};
