// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DroneMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Pawn.h"
#include "DronePawn.generated.h"

struct FLastInputValue
{
	float Forward = 0.0f;
	float Right = 0.0f;
	float Up = 0.0f;
	float Turn = 0.0f;
};

UCLASS()
class DRONEPROJECT_API ADronePawn : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* CollisionComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USkeletalMeshComponent* SkeletalMeshComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UCameraComponent* CameraComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UPawnMovementComponent* PawnMovementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UCurveFloat* CurveAccelerationDrone;

	// Скорость вращения
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drone|Controls")
	FRotator RotationRate = FRotator(45.0f, 45.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone|Tilt angle", meta = (ClampMin = 25.0f, UIMin = 25.0f, ClampMax = 75.0f, UIMax = 75.0f))
	float ForwardAngle = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone|Tilt angle", meta = (ClampMin = 25.0f, UIMin = 25.0f, ClampMax = 75.0f, UIMax = 75.0f))
	float RightAngle = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone|Camera", meta = (ClampMin = 1.0f, UIMin = 5.0f, ClampMax = 179.0f, UIMax = 179.0f))
	float CameraYawAngleLimit = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone|Camera", meta = (ClampMin = 1.0f, UIMin = 5.0f, ClampMax = 179.0f, UIMax = 179.0f))
	float CameraPitchAngleLimit = 45.0f;
	
	ADronePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE float GetDroneAcceleration() const { return Acceleration; }

	FORCEINLINE FLastInputValue GetLastInputValue() const { return LastInputValue; }

protected:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	TWeakObjectPtr<class UDroneMovementComponent> CachedDroneMovementComponent;

	FTimeline DroneTimeLine;
	float Acceleration = 1.0f;
	void DroneTimeLineUpdateComponent(float Output);

	FLastInputValue LastInputValue;

	void ChangeAngleDrone(FRotator TargetRotation);

	FORCEINLINE FRotator GetParallelGroundRotation() const { return FRotator(0.0f, GetControlRotation().Yaw, 0.0f); }
};
