// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "DroneMovementComponent.generated.h"

/**
 *  Логика наклона дрона во время движения - было решено отнести к компоненту передвижения
 */
UCLASS()
class DRONEPROJECT_API UDroneMovementComponent : public UPawnMovementComponent
{
	
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// метод отскока от любой стены
	void Rebound(const FHitResult &Hit);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone movement | Speed", meta = (ClampMin = 200.0f, UIMin = 200.0f, ClampMax = 2000.0f, UIMax = 2000.0f))
	float MaxSpeed = 1200.0f;

	// ускорение движения
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone movement | Speed", meta = (ClampMin = 1.0f, UIMin = 1.0f, ClampMax = 10.0f, UIMax = 10.0f))
	float Acceleration = 1.0f;

	// Торможение после отскока от стены
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone movement | Reflection", meta = (ClampMin = 0.2f, UIMin = 0.2f, ClampMax = 1.0f, UIMax = 1.0f))
	float ReboundForce = 0.8f;

	// Максимальная скорость для срабатывания отскока от стены
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone movement | Reflection", meta = (ClampMin = 100.0f, UIMin = 100.0f, ClampMax = 1000.0f, UIMax = 1000.0f))
	float MaxSpeedForReflection = 400.0f;

	// // отдельная настройка скорости поворота дрона (Не камеры)
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone | Tilt", meta = (ClampMin = 10.0f, UIMin = 10.0f, ClampMax = 300.0f, UIMax = 300.0f))
	// float SpeedRotationYaw = 50.0f;
	//
	// // отдельная настройка скорости Наклона дрона (Не камеры)
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone | Tilt", meta = (ClampMin = 10.0f, UIMin = 10.0f, ClampMax = 100.0f, UIMax = 100.0f))
	// float SpeedTurn = 50.0f;

	// ускорение наклона
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Tilt", meta = (ClampMin = 1.0f, UIMin = 1.0f, ClampMax = 10.0f, UIMax = 10.0f))
	float AccelerationTurn = 1.0f;

	// угол наклона вниз и вверх
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Tilt", meta = (ClampMin = 25.0f, UIMin = 25.0f, ClampMax = 75.0f, UIMax = 75.0f))
	float ForwardAngle = 45.0f;

	// угол наклона вправо и влево
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Tilt", meta = (ClampMin = 25.0f, UIMin = 25.0f, ClampMax = 75.0f, UIMax = 75.0f))
	float RightAngle = 25.0f;

	// маркер привязки поворота дрона к контроллеру
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drone | Camera")
	bool bLockMeshToCamera = false;

private:
	TWeakObjectPtr<class ADronePawn> CachedDrone;

	// шаблонный метод передвижения со скольжение вдоль стены
	void MoveComponent(float DeltaTime, FVector Delta, const FRotator NewRotation);

	// плавное обновление скорости
	void SetVelocityByInterp(float DeltaTime, const FVector InputVector);

	// плавный наклон дрона
	FRotator DroneTiltByInterp(float DeltaTime, const FVector InputVector) const;

	// константа хранящая текущий угол поворота дрона и нулевые координаты
	FRotator GetParallelGroundRotation() const;
	
	// // линейное обновление скорости (проблема - на концах ограничения дрон не может из-за большой дельты встать в предел)
	// void SetVelocityByClamp(float DeltaTime, const FVector InputVector);
	//
	// // линейный наклон дрона (проблема - не привязанный дрон к контроллеру по оси Y, дрожит при движении из-зи большой дельты между контроллером и дроном)
	// FRotator DroneTiltByClamp(float DeltaTime, const FVector InputVector) const;
};
