// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/DronePlayerController.h"
#include "Character/ThirdPersonCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Drone/DronePawn.h"
#include "Containers/Array.h"

ADronePlayerController::ADronePlayerController()
{
	CountSpawningDrones = BeginCountSpawningDrones;
}

void ADronePlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	GEngine->AddOnScreenDebugMessage(4, 100.0f, FColor::Red, FString::Printf(TEXT("CountSpawningDrones: %i | Count drones: %i"), CountSpawningDrones, SelfDrones.Num()));

    if(!IsValid(InPawn))
    	return;

	// Дополнительная проверка
	if(InPawn->IsA<AThirdPersonCharacter>())
	{
		CurrentTypePawn = ECurrentTypePawn::Character;
	}
	else if(InPawn->IsA<ADronePawn>())
	{
		CurrentTypePawn = ECurrentTypePawn::Drone;
	}

	if(!SelfCharacter.IsValid())
	{
		if(InPawn->IsA<AThirdPersonCharacter>())
		{
			SelfCharacter = StaticCast<AThirdPersonCharacter*>(InPawn);
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
		Possess(SelfDrones[IndexCurrentDrone].Get());
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
		Possess(SelfDrones[IndexCurrentDrone].Get());
	}
}

void ADronePlayerController::LaunchDrone()
{
	if(CurrentTypePawn != ECurrentTypePawn::Character)
		return;

	if(!IsValid(SubclassDronePawn))
		return;

	if(CountSpawningDrones <= 0)
		return;

	if(!SelfCharacter.IsValid())
		return;
	
	const FRotator SpawningRotation = SelfCharacter->GetActorRotation();

	const ADronePawn* DroneClass = GetDefault<ADronePawn>(SubclassDronePawn.Get());
	const FVector DroneCollisionExtend = IsValid(DroneClass) ? DroneClass->GetDroneCollisionExtend() : DefaultDroneCollisionExtend;
	
	const FVector StartSpawningOffset = FVector( SelfCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius() + DroneCollisionExtend.X, 0.0f, DroneCollisionExtend.Z );
	const FVector StartSpawningLocation = SelfCharacter->GetActorLocation() + SpawningRotation.RotateVector(StartSpawningOffset);
	const FVector SpawningLocation = StartSpawningLocation + SpawningRotation.RotateVector(SpawningOffset);

	if(IsObstacle(StartSpawningLocation, SpawningLocation, SpawningRotation, DroneCollisionExtend))
		return;

	ADronePawn* SpawnedDrone = StaticCast<ADronePawn*>(GetWorld()->SpawnActor(SubclassDronePawn, &SpawningLocation, &SpawningRotation));
	IndexCurrentDrone = SelfDrones.Add(SpawnedDrone);
	
	CountSpawningDrones--;
	Possess(SelfDrones[IndexCurrentDrone].Get());
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

	Possess(SelfDrones[IndexCurrentDrone].Get());
}

void ADronePlayerController::BackToPlayer()
{
	if(CurrentTypePawn != ECurrentTypePawn::Drone)
		return;
	
	if(!SelfCharacter.IsValid())
		return;

	Possess(SelfCharacter.Get());
}

void ADronePlayerController::SelfDestruct()
{
	if(CurrentTypePawn != ECurrentTypePawn::Drone)
		return;

	if(!SelfCharacter.IsValid())
		return;
	
	Possess(SelfCharacter.Get());
	
	SelfDrones[IndexCurrentDrone]->Destroy();
	SelfDrones.RemoveAt(IndexCurrentDrone);

	if(IndexCurrentDrone > 0)
		--IndexCurrentDrone;
}

bool ADronePlayerController::IsObstacle(const FVector StartSpawningLocation, const FVector SpawningLocation, const FRotator SpawningRotation, const FVector DroneSize) const
{
	FHitResult HitResult;
	
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(SelfCharacter.Get());
	
	FCollisionShape CollisionBox = FCollisionShape::MakeBox( DroneSize );
	
	return GetWorld()->SweepSingleByChannel(HitResult, StartSpawningLocation, SpawningLocation, SpawningRotation.Quaternion(), ECC_Visibility, CollisionBox, CollisionParams, FCollisionResponseParams::DefaultResponseParam);
}
