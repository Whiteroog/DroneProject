// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "DroneMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class DRONEPROJECT_API UDroneMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Rebound(const FHitResult &Hit);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone movement|Speed", meta = (ClampMin = 200.0f, UIMin = 200.0f, ClampMax = 2000.0f, UIMax = 2000.0f))
	float MaxSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone movement|Speed", meta = (ClampMin = 0.2f, UIMin = 0.2f, ClampMax = 1.5f, UIMax = 1.5f))
	float Acceleration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone movement|Reflection", meta = (ClampMin = 0.2f, UIMin = 0.2f, ClampMax = 1.5f, UIMax = 1.5f))
	float PushForce = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone movement|Reflection", meta = (ClampMin = 0.2f, UIMin = 0.2f, ClampMax = 1.5f, UIMax = 1.5f))
	float ValueTriggerReflection = 400.0f;

	// UPROPERTY(EditAnywhere, BlueprintReadOnly)
	// bool bIsGravity = true;

private:
	void SmoothChangeSpeed(FVector DesireVelocity, float DeltaTime);
	float AlphaSpeedDrone = 0.0f;
	FVector LastDesireVelocity = FVector::ZeroVector;

	float DecelerationAfterPush = 0.5f;
};
