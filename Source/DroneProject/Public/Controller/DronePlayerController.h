// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DronePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DRONEPROJECT_API ADronePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void SetPawn(APawn* InPawn) override;
	virtual void SetupInputComponent() override;

private:
	void LaunchDrone();
	void ConnectionToLaunchedDrone();
	void BackToPlayer();
	
	TSoftObjectPtr< class APawn > CurrentOwner;
	
	TSoftObjectPtr< class AThirdPersonCharacter > CharacterOwner;
	TSoftObjectPtr< class ADronePawn > DroneOwner;
};
