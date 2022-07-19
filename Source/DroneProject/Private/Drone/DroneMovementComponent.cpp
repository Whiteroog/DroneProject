// Fill out your copyright notice in the Description page of Project Settings.


#include "Drone/DroneMovementComponent.h"

void UDroneMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<ADronePawn>(), "UDroneMovementComponent ONLY work with ADronePawn class");
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

	// Получаем входящий вектор (и очищаем), нормализуя до единичной длины
	const FVector PendingInput = ConsumeInputVector().GetClampedToMaxSize(1.0f);

	// Если входящий вектор вниз,то винты не работают и дрон падает под действием гравитации
	const FVector NewVelocity = PendingInput * (GetLastInputVector().Z < -0.1 ? -GetGravityZ() : MaxSpeed);

	// Векторная интерполяция
	Velocity = FMath::VInterpTo(Velocity, NewVelocity, DeltaTime, SpeedAcceleration);

	// Шаблонное перемещение объекта с использованием функции скольжения
	const FVector Delta = Velocity * DeltaTime;
	if (!Delta.IsNearlyZero(1e-6f))
	{
		const FQuat Rot = UpdatedComponent->GetComponentQuat();
		
		FHitResult Hit(1.f);
		
		SafeMoveUpdatedComponent(Delta, Rot, true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			// 
			if(Velocity.Size() < MaxSpeedForReflection)
			{
				HandleImpact(Hit, DeltaTime, Delta);
				SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
			}
			else
			{
				Rebound(Hit);
			}
		}
	}

	UpdateComponentVelocity();

	GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Green, FString::Printf(TEXT("Speed: %f"), Velocity.Size()));
}

// Отскок
void UDroneMovementComponent::Rebound(const FHitResult &Hit)
{
	Velocity = Velocity - 2 * (Hit.ImpactNormal * (Velocity * Hit.ImpactNormal)); // Reflection Vector
	Velocity *= ReboundForce; // замедление
}