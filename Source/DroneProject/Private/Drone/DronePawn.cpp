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

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring arm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetRelativeLocation(FVector(50, 0, 0));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	CameraComponent->bUsePawnControlRotation = false;
	SpringArmComponent->bUsePawnControlRotation = true;

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

	GEngine->AddOnScreenDebugMessage(2, 1.0f, FColor::Red, FString::Printf(TEXT("Rotation: %f, %f, %f"), GetActorRotation().Pitch, GetControlRotation().Yaw, GetActorRotation().Roll));
}

void ADronePawn::MoveForward(float Value)
{
	if (Value != 0.0f && !CachedDroneMovementComponent->IsLanded())
	{
		ChangeRotation(GetWorld()->GetDeltaSeconds(), FRotator(ForwardAngle * Value, GetControlRotation().Yaw, GetActorRotation().Roll));
		
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
	}
}

void ADronePawn::LookUp(float Value)
{
	if(Value != 0.0f)
	{
		AddControllerPitchInput(RotationRate.Pitch * Value * GetWorld()->GetDeltaSeconds());
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
	if(CachedDroneMovementComponent->IsLanded())
	{
		TargetRotation.Yaw = GetActorRotation().Yaw;
	}
	
	FRotator NewRotation = FMath::Lerp(GetActorRotation(), TargetRotation, RotationAcceleration * DeltaTime);
	SetActorRotation(NewRotation);
}
