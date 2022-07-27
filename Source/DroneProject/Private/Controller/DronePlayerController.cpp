// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/DronePlayerController.h"
#include "Character/ThirdPersonCharacter.h"
#include "Drone/DronePawn.h"

void ADronePlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

    if(InPawn == nullptr)
    	return;
	
	CurrentOwner = InPawn;

	if(CurrentOwner->IsA<AThirdPersonCharacter>())
		CharacterOwner = Cast<AThirdPersonCharacter>(CurrentOwner.Get());
}

void ADronePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("LaunchDrone", EInputEvent::IE_Pressed, this, &ADronePlayerController::LaunchDrone);
	InputComponent->BindAction("ConnectionToLaunchedDrone", EInputEvent::IE_Pressed, this, &ADronePlayerController::ConnectionToLaunchedDrone);
	InputComponent->BindAction("BackToPlayer", EInputEvent::IE_Pressed, this, &ADronePlayerController::BackToPlayer);
}

void ADronePlayerController::LaunchDrone()
{
	if(DroneOwner.IsValid())
		return;
	
	if(!CurrentOwner->IsA<AThirdPersonCharacter>())
		return;
	
	if(!CharacterOwner.IsValid())
		return;

	const FVector SpawningOffset = FVector(200.0f, 0.0f, 50.0f);
	const FRotator SpawningRotation = CurrentOwner->GetActorRotation();
	const FVector SpawningLocation = CurrentOwner->GetActorLocation() + SpawningRotation.RotateVector(SpawningOffset);
	
	DroneOwner = GetWorld()->SpawnActor<ADronePawn>(SpawningLocation, SpawningRotation);
	Possess(DroneOwner.Get());
}

void ADronePlayerController::ConnectionToLaunchedDrone()
{
	if(!CurrentOwner->IsA<AThirdPersonCharacter>())
		return;
	
	if(!CharacterOwner.IsValid() || !DroneOwner.IsValid())
		return;
	
	Possess(DroneOwner.Get());
}

void ADronePlayerController::BackToPlayer()
{
	if(!CurrentOwner->IsA<ADronePawn>())
		return;
	
	if(!CharacterOwner.IsValid() || !DroneOwner.IsValid())
		return;
	
	Possess(CharacterOwner.Get());
}
