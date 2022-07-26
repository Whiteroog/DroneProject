// Fill out your copyright notice in the Description page of Project Settings.


#include "Drone/DronePawn.h"

#include "Drone/DroneMovementComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/PawnMovementComponent.h"

ADronePawn::ADronePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	CollisionComponent->SetBoxExtent(FVector(50, 50, 20));
	CollisionComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	RootComponent = CollisionComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static mesh"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal mesh"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	SkeletalMeshComponent->SetOwnerNoSee(true);
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SkeletalMeshComponent);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	CameraComponent->bUsePawnControlRotation = true;

	PawnMovementComponent = CreateDefaultSubobject<UPawnMovementComponent, UDroneMovementComponent>(TEXT("Movement component"));
	PawnMovementComponent->SetUpdatedComponent(CollisionComponent);

	checkf(PawnMovementComponent->IsA<UDroneMovementComponent>(), TEXT("Drone working ONLY with UDroneMovementComponent"));
	CachedDroneMovementComponent = StaticCast<UDroneMovementComponent*>(PawnMovementComponent);
}

void ADronePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LastControlRotation = GetSafeControlRotation();

	// Отображение вращения Roll дрона
	SetSafeControlRotation(
		FRotator(
			LastControlRotation.Pitch,
			LastControlRotation.Yaw,
			GetActorRotation().Roll));	// changing
}

void ADronePawn::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(FVector::ForwardVector, Value);
	}
}

void ADronePawn::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(FVector::RightVector, Value);
	}
}

void ADronePawn::MoveUp(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(FVector::UpVector, Value);
	}
}

void ADronePawn::Turn(float Value)
{
	const float DeltaTime = GetWorld()->GetDeltaSeconds();

	TurnValue = Value;
	
	if(Value != 0.0f)
	{
		const float SpeedTurn = RotationCameraRate.Yaw * Value * DeltaTime;
		AddControllerYawInput(SpeedTurn);

		// ограничение вращения контроллера по Yaw оси - если дрон не привязан к камере
		const float MinAngleDegrees = GetActorRotation().Yaw - CameraYawAngleLimit;
		const float MaxAngleDegrees = GetActorRotation().Yaw + CameraYawAngleLimit;

		const float YawControlRotation = FMath::ClampAngle(LastControlRotation.Yaw, MinAngleDegrees, MaxAngleDegrees);
		
		SetSafeControlRotation(
			FRotator(
				LastControlRotation.Pitch,
				YawControlRotation,		// changing
				LastControlRotation.Roll));
	}
}

void ADronePawn::LookUp(float Value)
{
	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	if(Value != 0.0f)
	{
		const float SpeedLookUp = RotationCameraRate.Pitch * Value * DeltaTime;
		AddControllerPitchInput(SpeedLookUp);

		// ограничение камеры по оси Pitch
		const float MinAngleDegrees = GetActorRotation().Pitch - CameraPitchAngleLimit;
		const float MaxAngleDegrees = GetActorRotation().Pitch + CameraPitchAngleLimit;
		
		const float PitchControlRotation = FMath::ClampAngle(LastControlRotation.Pitch, MinAngleDegrees, MaxAngleDegrees);
		SetSafeControlRotation(
			FRotator(
			PitchControlRotation,		// changing
			LastControlRotation.Yaw,
			LastControlRotation.Roll));
	}
}

void ADronePawn::SetSafeControlRotation(FRotator NewRotation) const
{
	if(!IsValid(Controller))
		return;

	Controller->SetControlRotation(NewRotation);
}

FRotator ADronePawn::GetSafeControlRotation() const
{
	return IsValid(Controller) ? Controller->GetControlRotation() : LastControlRotation;
}
