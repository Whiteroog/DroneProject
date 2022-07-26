// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/PlayerControllerDrone.h"

#include "Drone/DronePawn.h"

void APlayerControllerDrone::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	CachedDronePawn = Cast<ADronePawn>(InPawn);
}

void APlayerControllerDrone::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &APlayerControllerDrone::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &APlayerControllerDrone::MoveRight);
	InputComponent->BindAxis("MoveUp", this, &APlayerControllerDrone::MoveUp);
	InputComponent->BindAxis("Turn", this, &APlayerControllerDrone::Turn);
	InputComponent->BindAxis("LookUp", this, &APlayerControllerDrone::LookUp);
}

void APlayerControllerDrone::MoveForward(float Value)
{
	if (CachedDronePawn.IsValid())
	{
		CachedDronePawn->MoveForward(Value);
	}
}

void APlayerControllerDrone::MoveRight(float Value)
{
	if (CachedDronePawn.IsValid())
	{
		CachedDronePawn->MoveRight(Value);
	}
}

void APlayerControllerDrone::MoveUp(float Value)
{
	if (CachedDronePawn.IsValid())
	{
		CachedDronePawn->MoveUp(Value);
	}
}

void APlayerControllerDrone::Turn(float Value)
{
	if (CachedDronePawn.IsValid())
	{
		CachedDronePawn->Turn(Value);
	}
}

void APlayerControllerDrone::LookUp(float Value)
{
	if (CachedDronePawn.IsValid())
	{
		CachedDronePawn->LookUp(Value);
	}
}
