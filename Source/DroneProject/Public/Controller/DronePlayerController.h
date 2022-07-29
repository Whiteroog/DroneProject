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
	
public:
	ADronePlayerController();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Spawning")
	TSubclassOf< class ADronePawn > SubclassDronePawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Spawning")
	FVector SpawningOffset = FVector(200.0f, 0.0f, 50.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Spawning")
    FVector DefaultDroneCollisionExtend = FVector(50, 50, 20);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Spawning")
	int BeginCountSpawningDrones = 3;
	
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

	int CountSpawningDrones = 0;
	
	TWeakObjectPtr< class AThirdPersonCharacter > SelfCharacter;
	
	TArray< TWeakObjectPtr<class ADronePawn> > SelfDrones;
	int IndexCurrentDrone = 0;

	bool IsObstacle(const FVector StartSpawningLocation, const FVector SpawningLocation, const FRotator SpawningRotation, const FVector
	                DroneSize) const;
};
