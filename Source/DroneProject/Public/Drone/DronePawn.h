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
	
	ADronePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	FORCEINLINE float GetTurnValue() const { return TurnValue; }

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

	float TurnValue = 0.0f;
};
