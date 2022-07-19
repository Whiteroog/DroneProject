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

	// Используется TimeLine, чтобы поведение дрона было контролируемым
	if(IsValid(CurveAccelerationDrone))
	{
		FOnTimelineFloatStatic DroneTimeLineUpdate;
		DroneTimeLineUpdate.BindUObject(this, &ADronePawn::DroneTimeLineUpdateComponent);
		DroneTimeLine.AddInterpFloat(CurveAccelerationDrone,DroneTimeLineUpdate);
	}
}

// Called every frame
void ADronePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	DroneTimeLine.TickTimeline(DeltaTime);
	
	CameraComponent->SetWorldRotation(FRotator(GetControlRotation().Pitch, GetControlRotation().Yaw, GetActorRotation().Roll));
	
	GEngine->AddOnScreenDebugMessage(2, 1.0f, FColor::Red, FString::Printf(TEXT("Acceleration: %f"), GetDroneAcceleration()));
}

void ADronePawn::MoveForward(float Value)
{
	// Если изменения при вводе, то поведение дрона должно быть с задержкой
	if(LastInputValue.Forward != Value)
	{
		DroneTimeLine.PlayFromStart();
	}
	
	if (Value != 0.0f)
	{
		// Изменяем крен
		ChangeAngleDrone(FRotator(-ForwardAngle * Value, GetControlRotation().Yaw, GetActorRotation().Roll));

		// Но не изменяем вектор направления
		const FVector ForwardVector = GetParallelGroundRotation().RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, Value);
	}
	else
	{
		// Возвращение в стабильное параллельное положение
		ChangeAngleDrone(FRotator(0.0f, GetControlRotation().Yaw, GetActorRotation().Roll));
	}

	LastInputValue.Forward = Value;
}

void ADronePawn::MoveRight(float Value)
{
	if(LastInputValue.Right != Value)
	{
		DroneTimeLine.PlayFromStart();
	}
	
	if (Value != 0.0f)
	{
		ChangeAngleDrone(FRotator(GetActorRotation().Pitch, GetControlRotation().Yaw, RightAngle * Value));
		
		const FVector RightVector = GetParallelGroundRotation().RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
	else
	{
		ChangeAngleDrone(FRotator(GetActorRotation().Pitch, GetControlRotation().Yaw, 0.0f));
	}

	LastInputValue.Right = Value;
}

void ADronePawn::MoveUp(float Value)
{
	if(LastInputValue.Up != Value)
	{
		DroneTimeLine.PlayFromStart();
	}
	
	if (Value != 0.0f)
	{
		const FVector UpVector = GetParallelGroundRotation().RotateVector(FVector::UpVector);
		AddMovementInput(UpVector, Value);
	}

	LastInputValue.Up = Value;
}

void ADronePawn::Turn(float Value)
{
	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	if(Value != 0.0f)
	{
		const float SpeedTurn = RotationRate.Yaw * Value * DeltaTime;
		AddControllerYawInput(SpeedTurn);

		FRotator ControlRotation = GetControlRotation();
		const float MinAngleDegrees = GetActorRotation().Yaw - CameraYawAngleLimit;
		const float MaxAngleDegrees = GetActorRotation().Yaw + CameraYawAngleLimit;
		ControlRotation.Yaw = FMath::ClampAngle(ControlRotation.Yaw, MinAngleDegrees, MaxAngleDegrees);
		Controller->SetControlRotation(ControlRotation);
	}

	LastInputValue.Turn = Value;
}

void ADronePawn::LookUp(float Value)
{
	const float DeltaTime = GetWorld()->GetDeltaSeconds();
	
	if(Value != 0.0f)
	{
		AddControllerPitchInput(RotationRate.Pitch * Value * DeltaTime);
		
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

// Получение значение из Curve
void ADronePawn::DroneTimeLineUpdateComponent(float Output)
{
	Acceleration = Output;
}

// Изменение угла наклона дрона
void ADronePawn::ChangeAngleDrone(FRotator TargetRotation)
{
	// // Если на земле, то заблокировать единственное вращение дрона контроллером
	// if(CachedDroneMovementComponent->IsLanded())
	// {
	// 	TargetRotation.Yaw = GetActorRotation().Yaw; // Подключена только одна ось
	// }
	//
	SetActorRotation(FMath::Lerp(GetActorRotation(), TargetRotation, GetDroneAcceleration()));
}
