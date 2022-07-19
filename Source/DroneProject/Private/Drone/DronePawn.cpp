// Fill out your copyright notice in the Description page of Project Settings.


#include "Drone/DronePawn.h"

#include "Drone/DroneMovementComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/PawnMovementComponent.h"

// Sets default values
ADronePawn::ADronePawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	CollisionComponent->SetBoxExtent(FVector(50, 50, 20));
	CollisionComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	RootComponent = CollisionComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static mesh"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal mesh"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->SetRelativeLocation(FVector(20, 0, -4));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	CameraComponent->bUsePawnControlRotation = false;

	PawnMovementComponent = CreateDefaultSubobject<UPawnMovementComponent, UDroneMovementComponent>(TEXT("Movement component"));
	PawnMovementComponent->SetUpdatedComponent(CollisionComponent);

	CachedDroneMovementComponent = StaticCast<UDroneMovementComponent*>(PawnMovementComponent);
}

// Called when the game starts or when spawned
void ADronePawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADronePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraComponent->SetWorldRotation(FRotator(GetControlRotation().Pitch, GetControlRotation().Yaw, GetActorRotation().Roll));
}

void ADronePawn::MoveForward(float Value)
{
	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	if (Value != 0.0f && !CachedDroneMovementComponent->IsLanded())
	{
		// Изменяем крен
		ChangeAngleDrone(DeltaTime, FRotator(-ForwardAngle * Value, GetControlRotation().Yaw, GetActorRotation().Roll));

		// Но не изменяем вектор направления
		const FVector ForwardVector = GetParallelGroundRotation().RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, Value);
	}
	else
	{
		// Возвращение в стабильное параллельное положение
		ChangeAngleDrone(DeltaTime, FRotator(0.0f, GetControlRotation().Yaw, GetActorRotation().Roll));
	}
}

void ADronePawn::MoveRight(float Value)
{
	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	if (Value != 0.0f && !CachedDroneMovementComponent->IsLanded())
	{
		ChangeAngleDrone(DeltaTime, FRotator(GetActorRotation().Pitch, GetControlRotation().Yaw, RightAngle * Value));
		
		const FVector RightVector = GetParallelGroundRotation().RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
	else
	{
		ChangeAngleDrone(DeltaTime, FRotator(GetActorRotation().Pitch, GetControlRotation().Yaw, 0.0f));
	}
}

void ADronePawn::MoveUp(float Value)
{
	if (Value != 0.0f)
	{
		const FVector UpVector = GetParallelGroundRotation().RotateVector(FVector::UpVector);
		AddMovementInput(UpVector, Value);
	}
}

void ADronePawn::Turn(float Value)
{
	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	if(Value != 0.0f)
	{
		const float SpeedTurn = RotationRate.Yaw * Value * DeltaTime;
		AddControllerYawInput(SpeedTurn);

		// Когда летим и поворачивая камерой. Косметический наклон
		if(GetLastMovementInputVector().X != 0 || GetLastMovementInputVector().Y != 0)
			ChangeAngleDrone(DeltaTime, FRotator(GetActorRotation().Pitch, GetControlRotation().Yaw, RightAngle * Value));

		FRotator ControlRotation = GetControlRotation();
		const float MinAngleDegrees = GetActorRotation().Yaw - CameraYawAngleLimit;
		const float MaxAngleDegrees = GetActorRotation().Yaw + CameraYawAngleLimit;
		ControlRotation.Yaw = FMath::ClampAngle(ControlRotation.Yaw, MinAngleDegrees, MaxAngleDegrees);
		Controller->SetControlRotation(ControlRotation);
	}
}

void ADronePawn::LookUp(float Value)
{
	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	if(Value != 0.0f)
	{
		const float SpeedLookUp = RotationRate.Pitch * Value * DeltaTime;
		AddControllerPitchInput(SpeedLookUp);
		
		FRotator ControlRotation = GetControlRotation();
		const float MinAngleDegrees = GetActorRotation().Pitch - CameraPitchAngleLimit;
		const float MaxAngleDegrees = GetActorRotation().Pitch + CameraPitchAngleLimit;
		ControlRotation.Pitch = FMath::ClampAngle(ControlRotation.Pitch, MinAngleDegrees, MaxAngleDegrees);
		Controller->SetControlRotation(ControlRotation);
	}
}

// Called to bind functionality to input
void ADronePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ADronePawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADronePawn::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &ADronePawn::MoveUp);
	PlayerInputComponent->BindAxis("Turn", this, &ADronePawn::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ADronePawn::LookUp);
}

// Изменение угла наклона дрона
void ADronePawn::ChangeAngleDrone(float DeltaTime, FRotator TargetRotation)
{
	// Если на земле, то заблокировать единственное вращение дрона контроллером
	if(CachedDroneMovementComponent->IsLanded())
	{
		TargetRotation.Yaw = GetActorRotation().Yaw; // Подключена только одна ось
	}
	
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, Acceleration));
}
