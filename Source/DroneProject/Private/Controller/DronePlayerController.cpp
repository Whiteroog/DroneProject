// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/DronePlayerController.h"
#include "Character/ThirdPersonCharacter.h"
#include "Drone/DronePawn.h"
#include "Containers/Array.h"

void ADronePlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	GEngine->AddOnScreenDebugMessage(4, 100.0f, FColor::Red, FString::Printf(TEXT("IndexDrone: %i | Count drones: %i"), IndexCurrentDrone, SelfDrones.Num()));

    if(!IsValid(InPawn))
    	return;


	if(!SelfCharacter.IsValid())
	{
		if(InPawn->IsA<AThirdPersonCharacter>())
		{
			SelfCharacter = StaticCast<AThirdPersonCharacter*>(InPawn);
			CurrentTypePawn = ECurrentTypePawn::Character;
		}
	}
}

void ADronePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("ToNextDrone", EInputEvent::IE_Pressed, this, &ADronePlayerController::ToNextDrone);
	InputComponent->BindAction("ToPreviousDrone", EInputEvent::IE_Pressed, this, &ADronePlayerController::ToPreviousDrone);
	InputComponent->BindAction("LaunchDrone", EInputEvent::IE_Pressed, this, &ADronePlayerController::LaunchDrone);
	InputComponent->BindAction("ConnectionToLaunchedDrone", EInputEvent::IE_Pressed, this, &ADronePlayerController::ConnectionToLaunchedDrone);
	InputComponent->BindAction("BackToPlayer", EInputEvent::IE_Pressed, this, &ADronePlayerController::BackToPlayer);
	InputComponent->BindAction("SelfDestruct", EInputEvent::IE_Pressed, this, &ADronePlayerController::SelfDestruct);
}

void ADronePlayerController::ToNextDrone()
{
	if(CurrentTypePawn != ECurrentTypePawn::Drone)
		return;
	
	const int NextIndex = IndexCurrentDrone + 1;
	
	if(NextIndex < SelfDrones.Num())
	{
		IndexCurrentDrone = NextIndex;
		CurrentTypePawn = ECurrentTypePawn::Drone;
		Possess(SelfDrones[IndexCurrentDrone]);
	}
}

void ADronePlayerController::ToPreviousDrone()
{
	if(CurrentTypePawn != ECurrentTypePawn::Drone)
		return;
	
	const int PreviousIndex = IndexCurrentDrone - 1;
	
	if(0 <= PreviousIndex)
	{
		IndexCurrentDrone = PreviousIndex;
		CurrentTypePawn = ECurrentTypePawn::Drone;
		Possess(SelfDrones[IndexCurrentDrone]);
	}
}

void ADronePlayerController::LaunchDrone()
{
	if(CurrentTypePawn != ECurrentTypePawn::Character)
		return;

	if(SelfDrones.Num() >= CountSpawningDrones)
		return;

	if(!SelfCharacter.IsValid())
		return;
	
	const FRotator SpawningRotation = SelfCharacter->GetActorRotation();
	const FVector SpawningLocation = SelfCharacter->GetActorLocation() + SpawningRotation.RotateVector(SpawningOffset);

	ADronePawn* SpawnedDrone = Cast<ADronePawn>(GetWorld()->SpawnActor(SubclassDronePawn, &SpawningLocation, &SpawningRotation));
	IndexCurrentDrone = SelfDrones.Add(SpawnedDrone);

	CurrentTypePawn = ECurrentTypePawn::Drone;
	Possess(SelfDrones[IndexCurrentDrone]);
}

void ADronePlayerController::ConnectionToLaunchedDrone()
{
	if(CurrentTypePawn != ECurrentTypePawn::Character)
		return;
	
	if(SelfDrones.Num() == 0)
	{
		LaunchDrone();
		return;
	}

	CurrentTypePawn = ECurrentTypePawn::Drone;
	Possess(SelfDrones[IndexCurrentDrone]);
}

void ADronePlayerController::BackToPlayer()
{
	if(CurrentTypePawn != ECurrentTypePawn::Drone)
		return;
	
	if(!SelfCharacter.IsValid())
		return;

	CurrentTypePawn = ECurrentTypePawn::Character;
	Possess(SelfCharacter.Get());
}

void ADronePlayerController::SelfDestruct()
{
	if(CurrentTypePawn != ECurrentTypePawn::Drone)
		return;

	if(!SelfCharacter.IsValid())
		return;

	CurrentTypePawn = ECurrentTypePawn::Character;
	
	Possess(SelfCharacter.Get());
	
	SelfDrones[IndexCurrentDrone]->Destroy();
	SelfDrones.RemoveAt(IndexCurrentDrone);

	if(IndexCurrentDrone > 0)
		--IndexCurrentDrone;
}