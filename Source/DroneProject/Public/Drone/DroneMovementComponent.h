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

public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Rebound(const FHitResult &Hit);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone movement|Speed", meta = (ClampMin = 200.0f, UIMin = 200.0f, ClampMax = 2000.0f, UIMax = 2000.0f))
	float MaxSpeed = 1200.0f;

	// Торможение после отскока от стены
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone movement|Reflection", meta = (ClampMin = 0.2f, UIMin = 0.2f, ClampMax = 1.0f, UIMax = 1.0f))
	float ReboundForce = 0.8f;

	// Максимальная скорость для срабатывания отскока от стены
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone movement|Reflection", meta = (ClampMin = 100.0f, UIMin = 100.0f, ClampMax = 1000.0f, UIMax = 1000.0f))
	float MaxSpeedForReflection = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone|Speed", meta = (ClampMin = 1.0f, UIMin = 1.0f, ClampMax = 10.0f, UIMax = 10.0f))
	float Acceleration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone|Tilt angle", meta = (ClampMin = 25.0f, UIMin = 25.0f, ClampMax = 75.0f, UIMax = 75.0f))
	float ForwardAngle = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone|Tilt angle", meta = (ClampMin = 25.0f, UIMin = 25.0f, ClampMax = 75.0f, UIMax = 75.0f))
	float RightAngle = 25.0f;

private:
	TWeakObjectPtr<class ADronePawn> CachedDrone;

	void MoveComponent(float DeltaTime, FVector Delta, const FRotator NewRotation);
	void SetVelocity(float DeltaTime, const FVector InputVector);

	FRotator GetParallelGroundRotation() const;

	FRotator DroneTilt(float DeltaTime, const FVector InputVector) const;
};
