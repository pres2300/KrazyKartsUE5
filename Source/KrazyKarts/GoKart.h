// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GoKartMovementComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

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

	UPROPERTY(VisibleAnywhere)
	UGoKartMovementComponent* MovementComponent;

	UPROPERTY(ReplicatedUsing=OnRep_ServerState)
	FGoKartState ServerState;

	TArray<FGoKartMove> UnacknowledgedMoves;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove NewMove);

	void MoveForward(float Value);

	void MoveRight(float Value);

	void ClearAcknowledgedMoves(float LastMoveTime);

	UFUNCTION()
	void OnRep_ServerState();
};
