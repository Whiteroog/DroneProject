// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Drone/DronePawn.h"
#include "GameFramework/PlayerController.h"
#include "DronePlayerController.generated.h"

UENUM(BlueprintType)
enum class ECurrentTypePawn : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Character UMETA(DisplayName = "Character"),
	Drone UMETA(DisplayName = "Drone")
};

/**
 * 
 */
UCLASS()
class DRONEPROJECT_API ADronePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Spawning")
	TSubclassOf< class ADronePawn > SubclassDronePawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Spawning")
	FVector SpawningOffset = FVector(200.0f, 0.0f, 50.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Spawning")
    FVector DefaultDroneCollisionExtend = FVector(50, 50, 20);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Spawning")
	int CountSpawningDrones = 3;
	
	virtual void SetPawn(APawn* InPawn) override;
	virtual void SetupInputComponent() override;

private:
	void ToNextDrone();
	void ToPreviousDrone();
	void LaunchDrone();
	void ConnectionToLaunchedDrone();
	void BackToPlayer();
	void SelfDestruct();

	ECurrentTypePawn CurrentTypePawn = ECurrentTypePawn::Character;
	
	TWeakObjectPtr< class AThirdPersonCharacter > SelfCharacter;
	
	TArray< ADronePawn* > SelfDrones = TArray< ADronePawn* >();
	int IndexCurrentDrone = 0;

	bool IsObstacle(FVector ActorLocation, FVector SpawningLocation, FRotator SpawningRotation) const;
};
