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

	ADronePawn();

	FORCEINLINE FVector GetDroneCollisionExtend() const { return DroneCollisionExtend; }

	FRotator Internal_ConsumeControlInputRotation();
	FORCEINLINE FRotator Internal_GetLastControlInputRotation() const { return LastControlInputRotation; }

	FORCEINLINE FRotator GetLastControlRotation() const { return LastControlRotation; }

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drone | Collision")
	FVector DroneCollisionExtend = FVector(50, 50, 20);
	
	// Скорость вращения
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drone | Camera")
	FRotator RotationCameraRate = FRotator(45.0f, 45.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drone | Camera", meta = (ClampMin = 5.0f, UIMin = 5.0f, ClampMax = 179.0f, UIMax = 179.0f))
	float CameraYawAngleLimit = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drone | Camera", meta = (ClampMin = 5.0f, UIMin = 5.0f, ClampMax = 179.0f, UIMax = 179.0f))
	float CameraPitchAngleLimit = 45.0f;
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);
	void TurnRate(float Value);
	void LookUpRate(float Value);
	
	virtual void AddControllerYawInput(float Val) override;
	
private:
	TWeakObjectPtr<class UDroneMovementComponent> CachedDroneMovementComponent;

	// вводимое значение вращения контроллера
	FRotator ControlInputRotation = FRotator::ZeroRotator;
	FRotator LastControlInputRotation = FRotator::ZeroRotator;
	
	// последнее считанное значение вращения контроллера
	FRotator LastControlRotation = FRotator::ZeroRotator;
	
	// установить вращение контроллеру с проверкой на его валидность
	void SetSafeControlRotation(FRotator NewRotation) const;
	FRotator GetSafeControlRotation() const;
};
