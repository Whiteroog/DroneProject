// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* CollisionComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UPawnMovementComponent* MovementComponent;
	
	ADronePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
