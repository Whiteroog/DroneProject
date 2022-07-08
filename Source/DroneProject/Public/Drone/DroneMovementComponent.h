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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone movement|Speed", meta = (ClampMin = 1.0f, UIMin = 1.0f, ClampMax = 10.0f, UIMax = 10.0f))
	float SpeedAcceleration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone movement|Reflection", meta = (ClampMin = 0.2f, UIMin = 0.2f, ClampMax = 1.0f, UIMax = 1.0f))
	float ReboundForce = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone movement|Reflection", meta = (ClampMin = 100.0f, UIMin = 100.0f, ClampMax = 1000.0f, UIMax = 1000.0f))
	float ValueTriggerReflection = 400.0f;
	
};
