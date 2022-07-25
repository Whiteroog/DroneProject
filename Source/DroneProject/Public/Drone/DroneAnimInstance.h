// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DroneAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class DRONEPROJECT_API UDroneAnimInstance : public UAnimInstance
{
	
	GENERATED_BODY()

public:
	
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Propellers", meta = (ClampMin = 500.0f, UIMin = 500.0f, ClampMax = 1500.0f, UIMax = 1500.0f))
	float StabilizedSpeedRotationPropellers = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Propellers", meta = (ClampMin = 500.0f, UIMin = 500.0f, ClampMax = 20000.0f, UIMax = 2000.0f))
	float MaxSpeedRotationPropellers = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Propellers", meta = (ClampMin = 1.0f, UIMin = 1.0f, ClampMax = 10.0f, UIMax = 10.0f))
	float AccelerationRotationPropellers = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Propellers", meta = (ClampMin = 0.1f, UIMin = 0.1f, ClampMax = 1.0f, UIMax = 1.0f))
	float DecelerationRotationPropellers = 0.25f;

	UPROPERTY(BlueprintReadOnly)
	float SpeedRotationPropellers = 0.0f;

private:
	
	TWeakObjectPtr<class ADronePawn> CachedDrone;

	float GetSpeedRotationPropellers(float DeltaTime) const;
};
