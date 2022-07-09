// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DroneMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/Pawn.h"
#include "DronePawn.generated.h"

UCLASS()
class DRONEPROJECT_API ADronePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USpringArmComponent* SpringArmComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* CameraComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UPawnMovementComponent* PawnMovementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drone|Controls")
	FRotator RotationRate = FRotator(45.0f, 45.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drone|Tilt angle")
	float ForwardAngle = -45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drone|Tilt angle")
	float RightAngle = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone|Tilt angle", meta = (ClampMin = 1.0f, UIMin = 1.0f, ClampMax = 10.0f, UIMax = 10.0f))
	float RotationAcceleration = 1.0f;
	
	ADronePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void ChangeRotation(float DeltaTime, FRotator TargetRotation);
	TWeakObjectPtr<UDroneMovementComponent> CachedDroneMovementComponent;
};
