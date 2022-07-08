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

	const float SpeedForce = GetLastInputVector().Z < -0.1 ? -GetGravityZ() : MaxSpeed;
	const FVector SpeedVector = PendingInput * SpeedForce;
	Velocity = FMath::Lerp(Velocity, SpeedVector, SpeedAcceleration * DeltaTime);
	
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
	Velocity *= ReboundForce;
}