// Fill out your copyright notice in the Description page of Project Settings.


#include "Drone/DronePawn.h"

#include "Drone/DroneMovementComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

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

	// SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring arm"));
	// SpringArmComponent->SetupAttachment(RootComponent);
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->SetRelativeLocation(FVector(20, 0, -4));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	CameraComponent->bUsePawnControlRotation = false;
	// SpringArmComponent->bUsePawnControlRotation = true;

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
	
	GEngine->AddOnScreenDebugMessage(2, 1.0f, FColor::Red, FString::Printf(TEXT("Actor Rotation: %f, %f, %f"), GetActorRotation().Pitch, GetActorRotation().Yaw, GetActorRotation().Roll));
	GEngine->AddOnScreenDebugMessage(3, 1.0f, FColor::Red, FString::Printf(TEXT("Controller Rotation: %f, %f, %f"), GetControlRotation().Pitch, GetControlRotation().Yaw, GetControlRotation().Roll));
}

void ADronePawn::MoveForward(float Value)
{
	if (Value != 0.0f && !CachedDroneMovementComponent->IsLanded())
	{
		ChangeRotation(GetWorld()->GetDeltaSeconds(), FRotator(-ForwardAngle * Value, GetControlRotation().Yaw, GetActorRotation().Roll));
		
		// SetActorRotation(FRotator(ForwardAngle * Value, GetActorRotation().Yaw, GetActorRotation().Roll));

		const FRotator YawRotator = FRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		const FVector ForwardVector = YawRotator.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, Value);
	}
	else
	{
		ChangeRotation(GetWorld()->GetDeltaSeconds(), FRotator(0.0f, GetActorRotation().Yaw, GetActorRotation().Roll));
	}
}

void ADronePawn::MoveRight(float Value)
{
	if (Value != 0.0f && !CachedDroneMovementComponent->IsLanded())
	{
		ChangeRotation(GetWorld()->GetDeltaSeconds(), FRotator(GetActorRotation().Pitch, GetControlRotation().Yaw, RightAngle * Value));
		
		// SetActorRotation(FRotator(GetActorRotation().Pitch, GetActorRotation().Yaw, RightAngle * Value));

		const FRotator YawRotator = FRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		const FVector RightVector = YawRotator.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
	else
	{
		ChangeRotation(GetWorld()->GetDeltaSeconds(), FRotator(GetActorRotation().Pitch, GetControlRotation().Yaw, 0.0f));
	}
}

void ADronePawn::MoveUp(float Value)
{
	if (Value != 0.0f)
	{
		const FRotator YawRotator = FRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		const FVector UpVector = YawRotator.RotateVector(FVector::UpVector);
		AddMovementInput(UpVector, Value);
	}
}

void ADronePawn::Turn(float Value)
{
	if(Value != 0.0f)
	{
		AddControllerYawInput(RotationRate.Yaw * Value * GetWorld()->GetDeltaSeconds());
		
		FRotator ControlRotation = GetControlRotation();
		ControlRotation.Yaw = AngleClampWithPivot(ControlRotation.Yaw, GetActorRotation().Yaw,  CameraAngleYawLimit, CameraAngleYawLimit);
		Controller->SetControlRotation(ControlRotation);
	}
}

void ADronePawn::LookUp(float Value)
{
	if(Value != 0.0f)
	{
		AddControllerPitchInput(RotationRate.Pitch * Value * GetWorld()->GetDeltaSeconds());

		FRotator ControlRotation = GetControlRotation();
		ControlRotation.Pitch = AngleClampWithPivot(ControlRotation.Pitch,  GetActorRotation().Pitch, CameraAnglePitchLimit, CameraAnglePitchLimit);
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

void ADronePawn::ChangeRotation(float DeltaTime, FRotator TargetRotation)
{
	float Alpha = RotationAcceleration * DeltaTime;
	if(CachedDroneMovementComponent->IsLanded())
	{
		TargetRotation.Yaw = GetActorRotation().Yaw;
		Alpha = CachedDroneMovementComponent->GetBraking();
	}
	
	FRotator NewRotation = FMath::Lerp(GetActorRotation(), TargetRotation, Alpha);
	SetActorRotation(NewRotation);
}

// Pivot - точка отклонения
// Left - отклонение влево от Pivot
// Right - отклонение вправо от Pivot
float ADronePawn::AngleClampWithPivot(float Angle, float Pivot, float Left, float Right)
{
	Angle = FRotator::ClampAxis(Angle);
	Pivot = FRotator::ClampAxis(Pivot);

	Left = FMath::Abs(Left);
	Right = FMath::Abs(Right);
	
	bool LeftDeviationIsTurned = false;
	bool RightDeviationIsTurned = false;

	bool AngleIsTurned = false;

	// если разница между текущем углом поворота и фиксированным будет больше диапазона для Clamp, то Angle совершил оборот 
	if(FMath::Abs(Angle - Pivot) > FMath::Abs(Left + Right))
	{
		AngleIsTurned = true;
	}
	
	float LeftDeviation = Pivot - Left;
	float RightDeviation = Pivot + Right;
	
	if(FRotator::ClampAxis(LeftDeviation) != LeftDeviation)
	{
		LeftDeviationIsTurned = true;
	}
	LeftDeviation = FRotator::ClampAxis(LeftDeviation);

	if(FRotator::ClampAxis(RightDeviation) != RightDeviation)
	{
		RightDeviationIsTurned = true;
	}
	RightDeviation = FRotator::ClampAxis(RightDeviation);

	float Delta = Angle - LeftDeviation;

	// Если Pivot справа 0, но ограничение слева, то добавить 360, пока угол не стал левее 0
	// Если Pivot слева 0 с ограничением, но угол правее 0, то добавить 360
	Delta += (LeftDeviationIsTurned && !AngleIsTurned) | (!LeftDeviationIsTurned && AngleIsTurned) ? 360.0f : 0.0f;
	
	if(Delta < 0.0f)
	{
		return LeftDeviation;
	}

	Delta = RightDeviation - Angle;

	// Если Pivot слева 0, но ограничение справа, то добавить 360, пока угол не стал правее 0
	// Если Pivot справа 0 с ограничением, но угол левее 0, то добавить 360
	Delta += (RightDeviationIsTurned && !AngleIsTurned) | (!RightDeviationIsTurned && AngleIsTurned) ? 360.0f : 0.0f;

	if(Delta < 0.0f)
	{
		return RightDeviation;
	}
	
	return Angle;
}