// Fill out your copyright notice in the Description page of Project Settings.

#include "Drone/DronePawn.h"
#include "Drone/DroneMovementComponent.h"

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

	SetVelocity(DeltaTime);
	
	// Шаблонное перемещение объекта с использованием функции скольжения
	const FVector Delta = Velocity * DeltaTime;
	const FRotator NewRotation = UpdatedComponent->GetComponentRotation();
	MoveComponent(DeltaTime, Delta,NewRotation);

	UpdateComponentVelocity();

	GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Green, FString::Printf(TEXT("Speed: %f"), Velocity.Size()));
}

// Отскок
void UDroneMovementComponent::Rebound(const FHitResult &Hit)
{
	Velocity = Velocity - 2 * (Hit.ImpactNormal * (Velocity * Hit.ImpactNormal)); // Reflection Vector
	Velocity *= ReboundForce; // замедление
}

void UDroneMovementComponent::MoveComponent(float DeltaTime, const FVector Delta, const FRotator NewRotation)
{
	if (!Delta.IsNearlyZero(1e-6f))
	{
		FHitResult Hit(1.f);
		
		SafeMoveUpdatedComponent(Delta, NewRotation, true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			if(Velocity.Size() < MaxSpeedForReflection)
			{
				HandleImpact(Hit, DeltaTime, Delta);
				SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);

				// когда приземлились, скорость Z должна обнулиться (долго спадает)
				if(0.7f < Hit.ImpactNormal.Z && Hit.ImpactNormal.Z <= 1.0f)
					Velocity.Z = 0.0f;
			}
			else
			{
				Rebound(Hit);
			}
		}
	}
}

void UDroneMovementComponent::SetVelocity(float DeltaTime)
{
	// Получаем входящий вектор (и очищаем), нормализуя до единичной длины
	const FVector PendingInput = ConsumeInputVector().GetClampedToMaxSize(1.0f);

	// Если входящий вектор вниз,то винты не работают и дрон падает под действием гравитации
	const FVector NewVelocity = PendingInput * (GetLastInputVector().Z < -0.1 ? -GetGravityZ() : MaxSpeed);
	
	// Векторная интерполяция
	Velocity = FMath::VInterpTo(Velocity, NewVelocity, DeltaTime,CachedDrone->GetDroneAcceleration());
}