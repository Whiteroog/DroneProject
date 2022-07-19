// Fill out your copyright notice in the Description page of Project Settings.

#include "Drone/DronePawn.h"
#include "Drone/DroneMovementComponent.h"

void UDroneMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
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

	
	if(CachedDrone->GetLastInputValue().Up > 0.1)
		bIsLanded = false;

	// Если входящий вектор вниз,то винты не работают и дрон падает под действием гравитации
	const FVector NewVelocity = PendingInput * (CachedDrone->GetLastInputValue().Up < -0.1 ? -GetGravityZ() : MaxSpeed);
	
	// Векторная интерполяция
	Velocity = FMath::Lerp(Velocity, NewVelocity, CachedDrone->GetDroneAcceleration());

	// Если на земле, то не пытаемся пролететь сквозь нее
	if(bIsLanded)
		Velocity.Z = 0.0f;

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

				if(FMath::IsNearlyZero(Hit.ImpactNormal.Z, 0.1f))
					bIsLanded = true;
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