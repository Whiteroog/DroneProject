// Fill out your copyright notice in the Description page of Project Settings.

#include "Drone/DroneMovementComponent.h"

#include "Drone/DronePawn.h"

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
	SetVelocityByInterp(DeltaTime, PendingInput);
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

	// Чтобы текущие значение не высчитывалось на пределах
	if(Velocity.Equals(NewVelocity, 0.1f))
	{
		Velocity = NewVelocity;
		return;
	}

	// Векторная интерполяция
	Velocity = FMath::VInterpTo(Velocity, NewVelocity, DeltaTime, SpeedVelocity); 
}

FRotator UDroneMovementComponent::DroneTiltByInterp(float DeltaTime, const FVector InputVector) const
{
	FRotator Result = FRotator::ZeroRotator;
	
	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation();

	// значение поворота
	const float TurnValue = CachedDrone->GetTurnValue();

	// Изначальное параллельное положение
	FRotator TargetRotation = GetParallelGroundRotation();

	// Нормализация [-180; 180]
	CurrentRotation.Normalize();
	TargetRotation.Normalize();

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

	// Чтобы текущие значение не высчитывалось на пределах
	if(CurrentRotation.Equals(TargetRotation, 0.1f))
	{
		return TargetRotation;
	}

	Result = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, SpeedTurn);

	// привязка дрона к контроллеру
	if(bLockMeshToCamera)
	{
		Result.Yaw = GetParallelGroundRotation().Yaw;
	}
	else
	{
		// Чтобы поворот дрона к камере проходил быстрее
		Result.Yaw = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, SpeedRotationYaw).Yaw;
	}

	return Result;
}

FRotator UDroneMovementComponent::GetParallelGroundRotation() const
{
	return FRotator(0.0f, CachedDrone->GetLastControlRotation().Yaw, 0.0f);
}