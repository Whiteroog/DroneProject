// Fill out your copyright notice in the Description page of Project Settings.

#include "Drone/DronePawn.h"
#include "Drone/DroneMovementComponent.h"

#include "Kismet/GameplayStatics.h"

void UDroneMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<ADronePawn>(), TEXT("UDroneMovementComponent working ONLY with ADronePawn"));
	CachedDrone = StaticCast<ADronePawn*>(GetOwner());
}

void UDroneMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	if (ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Получаем входящий голый вектор (и очищаем), нормализуя до единичной длины
	const FVector PendingInput = ConsumeInputVector().GetClampedToMaxSize(1.0f);

	// Изменение направление и вращение дрона
	SetVelocityByClamp(DeltaTime, PendingInput);
	const FRotator NewRotation = DroneTiltByClamp(DeltaTime, PendingInput);

	// Шаблонное перемещение объекта с использованием функции скольжения
	const FVector Delta = Velocity * DeltaTime;
	MoveComponent(DeltaTime, Delta, NewRotation);

	UpdateComponentVelocity();

	GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Green, FString::Printf(TEXT("Speed: %f"), Velocity.Size()));
}

// Отскок
void UDroneMovementComponent::Rebound(const FHitResult& Hit)
{
	Velocity = Velocity - 2 * (Hit.ImpactNormal * (Velocity * Hit.ImpactNormal)); // Reflection Vector
	Velocity *= ReboundForce; // замедление
}

void UDroneMovementComponent::MoveComponent(float DeltaTime, const FVector Delta, const FRotator NewRotation)
{
	// Не двигаемся если есть небольшое отклонение или не поворачиваем камеру
	if (Delta.IsNearlyZero() && NewRotation.IsNearlyZero() && !CachedDrone->GetTurnValue())
	{
		return;
	}
	
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Delta, NewRotation, true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		if (Velocity.Size() < MaxSpeedForReflection)
		{
			HandleImpact(Hit, DeltaTime, Delta);
			SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);

			// когда приземлились, скорость Z должна обнулиться (долго спадает)
			if (0.7f < Hit.ImpactNormal.Z && Hit.ImpactNormal.Z <= 1.0f)
				Velocity.Z = 0.0f;
		}
		else
		{
			Rebound(Hit);
		}
	}
}

void UDroneMovementComponent::SetVelocityByInterp(float DeltaTime, const FVector InputVector)
{
	// Поворачиваем вектор
	const FVector RotatedInputVector = GetParallelGroundRotation().RotateVector(InputVector);

	// Если входящий вектор вниз,то винты не работают и дрон падает под действием гравитации
	const FVector NewVelocity = RotatedInputVector * (GetLastInputVector().Z < -0.1 ? -GetGravityZ() : MaxSpeed);

	// Векторная интерполяция
	Velocity = FMath::VInterpTo(Velocity, NewVelocity, DeltaTime, Acceleration); 
}

// Строгий набор скорости (второй метод)
// Поворачиваем вектор
void UDroneMovementComponent::SetVelocityByClamp(float DeltaTime, const FVector InputVector)
{
	const float Smoothing = Acceleration * DeltaTime;
	
	const FVector Direction = InputVector == FVector::ZeroVector ?
		Velocity.GetSafeNormal() * -1.0f :								// Чтобы тормозить если есть ускорение
		GetParallelGroundRotation().RotateVector(InputVector);

	if(Velocity.IsNearlyZero(1.0f) && InputVector == FVector::ZeroVector)
	{
		Velocity = FVector::ZeroVector;
		return;
	}
	
	const float CurrentMaxSpeed = GetLastInputVector().Z < -0.1 ? -GetGravityZ() : MaxSpeed;
	const FVector Delta = Direction * CurrentMaxSpeed * Smoothing;

	const FVector TargetVelocity = Velocity + Delta;

	Velocity = TargetVelocity.GetClampedToSize(0.0f, CurrentMaxSpeed);
}

FRotator UDroneMovementComponent::GetParallelGroundRotation() const
{
	return FRotator(0.0f, CachedDrone->GetControlRotation().Yaw, 0.0f);
}

FRotator UDroneMovementComponent::DroneTiltByInterp(float DeltaTime, const FVector InputVector) const
{
	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation();

	if(bLockMeshToCamera)
	{
		CurrentRotation.Yaw = GetParallelGroundRotation().Yaw;
	}
	
	const float TurnValue = CachedDrone->GetTurnValue();

	// Изначальное параллельное положение
	FRotator TargetRotation = GetParallelGroundRotation();

	if (InputVector.X != 0.0f)
		TargetRotation.Pitch = -ForwardAngle * InputVector.X;

	if (InputVector.Y != 0.0f)
		TargetRotation.Roll = RightAngle * InputVector.Y;

	if (TurnValue != 0.0f)
	{
		// Когда летим и поворачивая камерой. Косметический наклон
		if (InputVector.X != 0 || InputVector.Y != 0)
			TargetRotation.Roll = RightAngle * TurnValue;
	}

	// GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Red, FString::Printf(TEXT("Rotation: %f | %f"), CurrentRotation.Yaw, TargetRotation.Yaw));
	return FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, AccelerationTurn);
}

FRotator UDroneMovementComponent::DroneTiltByClamp(float DeltaTime, const FVector InputVector) const
{
	const float Smoothing = AccelerationTurn * DeltaTime;

	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation();

	const float TurnValue = CachedDrone->GetTurnValue();

	if(bLockMeshToCamera)
	{
		CurrentRotation.Yaw = GetParallelGroundRotation().Yaw;
	}

	const float DeltaYaw = CachedDrone->GetControlRotation().Yaw - FRotator::ClampAxis(CurrentRotation.Yaw);

	GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Red, FString::Printf(TEXT("DeltaYaw: %f | %f - %f"), DeltaYaw, CachedDrone->GetControlRotation().Yaw, FRotator::ClampAxis(CurrentRotation.Yaw)));
	
	FRotator Direction = FRotator(
		
		InputVector.X == 0.0f ? // Если есть нажатие вперед
		
			FMath::IsNearlyZero(CurrentRotation.Pitch, 0.5f) ?
				0.0f :
				CurrentRotation.Pitch / FMath::Abs(CurrentRotation.Pitch) * -1.0f : // нормализованный обратный X
		
		-InputVector.X,
		
		FMath::IsNearlyZero(DeltaYaw) ? 0.0f : DeltaYaw / FMath::Abs(DeltaYaw),
		
		(InputVector.Y == 0.0f) ? 
		
			FMath::IsNearlyZero(CurrentRotation.Roll, 0.5f) ?
				0.0f :
				CurrentRotation.Roll / FMath::Abs(CurrentRotation.Roll) * -1.0f : // нормализованный обратный Y
		
		InputVector.Y
		
		);
	
	if(TurnValue)
	{
		if(InputVector.X != 0 || InputVector.Y != 0) // поворот камеры во время движения
		{
			Direction.Roll = TurnValue;
		}
	}

	if(CurrentRotation.Equals(GetParallelGroundRotation(), 1.0f) && InputVector == FVector::ZeroVector)
		return GetParallelGroundRotation();

	const FRotator Delta = Direction * SpeedTurn * Smoothing;

	FRotator TargetRotation = CurrentRotation + Delta;

	// ограничения по наклону
	const float MinAngleDegreesPitch = GetParallelGroundRotation().Pitch - ForwardAngle;
	const float MaxAngleDegreesPitch = GetParallelGroundRotation().Pitch + ForwardAngle;

	const float MinAngleDegreesRoll = GetParallelGroundRotation().Roll - RightAngle;
	const float MaxAngleDegreesRoll = GetParallelGroundRotation().Roll + RightAngle;

	TargetRotation.Pitch = FMath::ClampAngle(TargetRotation.Pitch, MinAngleDegreesPitch, MaxAngleDegreesPitch);
	TargetRotation.Roll = FMath::ClampAngle(TargetRotation.Roll, MinAngleDegreesRoll, MaxAngleDegreesRoll);

	// GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Red, FString::Printf(TEXT("Rotation: %f | %f"), TargetRotation.Pitch, TargetRotation.Roll));
	
	return TargetRotation;
}