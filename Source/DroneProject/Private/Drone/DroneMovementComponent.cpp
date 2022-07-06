// Fill out your copyright notice in the Description page of Project Settings.


#include "Drone/DroneMovementComponent.h"

void UDroneMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	if (ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const FVector PendingInput = GetPendingInputVector().GetClampedToMaxSize(1.0f);
	ConsumeInputVector();

	if (!PendingInput.IsNearlyZero())
	{
		SmoothChangeSpeed(PendingInput * MaxSpeed, DeltaTime);
	}
	else if (!Velocity.IsNearlyZero())
	{
		SmoothChangeSpeed(FVector::ZeroVector, DeltaTime);
	}
	else
	{
		AlphaSpeedDrone = 0.0f;
		Velocity = FVector::ZeroVector;
	}
	
	const FVector Delta = Velocity * DeltaTime;
	if (!Delta.IsNearlyZero(1e-6f))
	{
		const FQuat Rot = UpdatedComponent->GetComponentQuat();
		
		FHitResult Hit(1.f);
		
		SafeMoveUpdatedComponent(Delta, Rot, true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			if(Velocity.Size() < ValueTriggerReflection)
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

void UDroneMovementComponent::Rebound(const FHitResult &Hit)
{
	Velocity = Velocity - 2 * (Hit.ImpactNormal * (Velocity * Hit.ImpactNormal)); // Reflection Vector
	Velocity *= PushForce;
	AlphaSpeedDrone *= DecelerationAfterPush;
}

void UDroneMovementComponent::SmoothChangeSpeed(FVector DesireVelocity, float DeltaTime)
{
	if (LastDesireVelocity != DesireVelocity)
	{
		LastDesireVelocity = DesireVelocity;
		AlphaSpeedDrone = 0.0f;
	}
	AlphaSpeedDrone += Acceleration * DeltaTime;
	AlphaSpeedDrone = FMath::Clamp(AlphaSpeedDrone, 0.0f, 1.0f);
	Velocity = FMath::Lerp(Velocity, DesireVelocity, AlphaSpeedDrone);
}
