// Fill out your copyright notice in the Description page of Project Settings.


#include "Drone/DroneAnimInstance.h"
#include "Drone/DronePawn.h"

void UDroneAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	checkf(TryGetPawnOwner()->IsA<ADronePawn>(), TEXT("UDroneAnimInstance working ONLY with ADronePawn"));
	CachedDrone = StaticCast<ADronePawn*>(TryGetPawnOwner());
}

// метод обновления пропеллеров работает в редакторе )
void UDroneAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// без этой проверки - редактор крашится
	if(IsValid(CachedDrone.Get()))
	{
		SpeedRotationPropellers = GetSpeedRotationPropellers(DeltaSeconds);
	}
}

float UDroneAnimInstance::GetSpeedRotationPropellers(float DeltaTime) const
{
	// вращение пропеллеров зависит от движения = значения ввода
	const FVector InputVector = CachedDrone->GetLastMovementInputVector();
	const float SpeedPropellers = InputVector.IsNearlyZero() ? StabilizedSpeedRotationPropellers : MaxSpeedRotationPropellers;

	float NewSpeedRotationPropellers = SpeedPropellers * DeltaTime;

	// если опускаемся
	if(InputVector.Z < -0.1f)
	{
		// замедление текущей скорости
		NewSpeedRotationPropellers *= DecelerationRotationPropellers;
	}

	GEngine->AddOnScreenDebugMessage(3, 1.0f, FColor::Blue, FString::Printf(TEXT("SpeedPropellers: %f"), NewSpeedRotationPropellers));

	// плавное вращение
	return FMath::FInterpTo(SpeedRotationPropellers, NewSpeedRotationPropellers, DeltaTime, AccelerationRotationPropellers);
}
