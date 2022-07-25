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
	const FRotator NewRotation = DroneTiltByInterp(DeltaTime, PendingInput);

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
		// если движемся меньше заданной скорости, то можем скользить, иначе отталкиваемся
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
	// Поворачиваем вектор, так как на вход подается вектор не сонаправленный
	const FVector RotatedInputVector = GetParallelGroundRotation().RotateVector(InputVector);

	// Если входящий вектор вниз,то винты не работают и дрон падает под действием гравитации
	const FVector NewVelocity = RotatedInputVector * (GetLastInputVector().Z < -0.1 ? -GetGravityZ() : MaxSpeed);

	// Векторная интерполяция
	Velocity = FMath::VInterpTo(Velocity, NewVelocity, DeltaTime, Acceleration); 
}

// Строгий набор скорости (второй метод)
void UDroneMovementComponent::SetVelocityByClamp(float DeltaTime, const FVector InputVector)
{
	const float Smoothing = Acceleration * DeltaTime;
	
	const FVector Direction = InputVector == FVector::ZeroVector ?
		Velocity.GetSafeNormal() * -1.0f :								// обратный вектор ускорения, чтобы тормозить
		GetParallelGroundRotation().RotateVector(InputVector);			// поворачиваем не сонаправленный вектор

	// если остановились и не двигаемся
	if(Velocity.IsNearlyZero(1.0f) && InputVector == FVector::ZeroVector)
	{
		Velocity = FVector::ZeroVector;
		return;
	}
	
	const float CurrentMaxSpeed = GetLastInputVector().Z < -0.1 ? -GetGravityZ() : MaxSpeed;
	const FVector VelocityDelta = Direction * CurrentMaxSpeed;
	const FVector Delta = VelocityDelta * Smoothing;

	// плавное ускорение
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

	// привязка дрона к контроллеру
	if(bLockMeshToCamera)
	{
		CurrentRotation.Yaw = GetParallelGroundRotation().Yaw;
	}

	// значение поворота
	const float TurnValue = CachedDrone->GetTurnValue();

	// Изначальное параллельное положение
	FRotator TargetRotation = GetParallelGroundRotation();

	if (InputVector.X != 0.0f)
		TargetRotation.Pitch = -ForwardAngle * InputVector.X;

	if (InputVector.Y != 0.0f)
		TargetRotation.Roll = RightAngle * InputVector.Y;

	if (TurnValue != 0.0f)
	{
		// Когда летим, поворачивая камерой - даем угол наклона поворота. <Косметический наклон>
		if (InputVector.X != 0 || InputVector.Y != 0)
			TargetRotation.Roll = RightAngle * TurnValue;
	}

	// GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Red, FString::Printf(TEXT("Rotation: %f | %f"), CurrentRotation.Yaw, TargetRotation.Yaw));
	return FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, AccelerationTurn);
}

// Рекомендуется выставить LockMeshToCamera = true, иначе дрон дрожит (проблема)
FRotator UDroneMovementComponent::DroneTiltByClamp(float DeltaTime, const FVector InputVector) const
{
	const float Smoothing = AccelerationTurn * DeltaTime;

	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation();

	const float TurnValue = CachedDrone->GetTurnValue();

	if(bLockMeshToCamera)
	{
		CurrentRotation.Yaw = GetParallelGroundRotation().Yaw;
	}

	// нормализуем ротаторы [-180; 180]
	const float TargetYawRotation = FRotator::NormalizeAxis(CachedDrone->GetControlRotation().Yaw);
	const float CurrentYawRotation = FRotator::NormalizeAxis(CurrentRotation.Yaw);

	// получаем знак направления поворота оси Y
	const float DeltaYaw = FRotator::NormalizeAxis(TargetYawRotation - CurrentYawRotation);

	GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Red, FString::Printf(TEXT("DeltaYaw: %f | %f - %f"), DeltaYaw, TargetYawRotation, CurrentYawRotation));

	// направление единичной длины ( u / |u| )
	FRotator Direction = FRotator(
		
		InputVector.X != 0.0f ? // Если есть нажатие вперед

			-InputVector.X :

			// иначе получаем знак обратного направления, чтобы выровняться
			(FMath::IsNearlyZero(CurrentRotation.Pitch, 0.5f) ?
				0.0f :
				CurrentRotation.Pitch / FMath::Abs(CurrentRotation.Pitch) * -1.0f), // нормализованный обратный X
		
		FMath::IsNearlyZero(DeltaYaw, 1.0f) ? 0.0f : DeltaYaw / FMath::Abs(DeltaYaw),
		
		(InputVector.Y != 0.0f) ? 

			InputVector.Y :
		
			(FMath::IsNearlyZero(CurrentRotation.Roll, 0.5f) ?
				0.0f :
				CurrentRotation.Roll / FMath::Abs(CurrentRotation.Roll) * -1.0f) // нормализованный обратный Y
				
		);
	
	if(TurnValue)
	{
		if(InputVector.X != 0 || InputVector.Y != 0)
		{
			Direction.Roll = TurnValue;
		}
	}

	// если выровнялись и не двигаемся
	if(CurrentRotation.Equals(GetParallelGroundRotation(), 1.0f) && InputVector == FVector::ZeroVector)
		return GetParallelGroundRotation();
	
	const FRotator RotationVelocity = FRotator(
		Direction.Pitch * SpeedTurn,
		Direction.Yaw * SpeedRotationYaw, // если вращение дрона по оси Z не привязан к камере, то вращение должен быть привязан к собственной скорости
		Direction.Roll * SpeedTurn
	);

	const FRotator Delta = RotationVelocity * Smoothing;

	FRotator TargetRotation = CurrentRotation + Delta;

	// ограничения по наклону
	const float MinAngleDegreesPitch = GetParallelGroundRotation().Pitch - ForwardAngle;
	const float MaxAngleDegreesPitch = GetParallelGroundRotation().Pitch + ForwardAngle;

	const float MinAngleDegreesRoll = GetParallelGroundRotation().Roll - RightAngle;
	const float MaxAngleDegreesRoll = GetParallelGroundRotation().Roll + RightAngle;

	TargetRotation.Pitch = FMath::ClampAngle(TargetRotation.Pitch, MinAngleDegreesPitch, MaxAngleDegreesPitch);
	TargetRotation.Roll = FMath::ClampAngle(TargetRotation.Roll, MinAngleDegreesRoll, MaxAngleDegreesRoll);
	
	return TargetRotation;
}