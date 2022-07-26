// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DroneMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "DronePawn.generated.h"

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

	// Скорость вращения
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Camera")
	FRotator RotationCameraRate = FRotator(45.0f, 45.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Camera", meta = (ClampMin = 5.0f, UIMin = 5.0f, ClampMax = 179.0f, UIMax = 179.0f))
	float CameraYawAngleLimit = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Camera", meta = (ClampMin = 5.0f, UIMin = 5.0f, ClampMax = 179.0f, UIMax = 179.0f))
	float CameraPitchAngleLimit = 45.0f;

	virtual void Tick(float DeltaTime) override;
	
	ADronePawn();

	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	
	FORCEINLINE float GetTurnValue() const { return TurnValue; }
	FORCEINLINE FRotator GetLastControlRotation() const { return LastControlRotation; }
	
private:
	TWeakObjectPtr<class UDroneMovementComponent> CachedDroneMovementComponent;

	// вводимое значение Yaw вращения
	float TurnValue = 0.0f;

	// последнее вводимое значение вращения контроллера
	FRotator LastControlRotation = FRotator::ZeroRotator;

	// установить вращение контроллеру с проверкой на его валидность
	void SetSafeControlRotation(FRotator NewRotation) const;
	FRotator GetSafeControlRotation() const;
};
