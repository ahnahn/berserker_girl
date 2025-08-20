#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoScreenScalerComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BERSERKER_GIRL_API UAutoScreenScalerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAutoScreenScalerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float AccumulatedTime = 0.f;
	int32 FrameCount = 0;
	float AverageFPS = 0.f;
	float TimeSinceLastAdjust = 0.f;

	int32 CurrentScreenPercentage = 100;

	// === 설정 ===
	UPROPERTY(EditAnywhere, Category = "Auto Scaling")
		float TargetFPS = 59.f;

	UPROPERTY(EditAnywhere, Category = "Auto Scaling")
		float FPSMargin = 2.f;

	UPROPERTY(EditAnywhere, Category = "Auto Scaling")
		float CriticalLowFPS = 55.f;

	UPROPERTY(EditAnywhere, Category = "Auto Scaling")
		float FPSCheckIntervalSeconds = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Auto Scaling")
		float MinAdjustmentInterval = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Auto Scaling")
		int32 AggressiveDropStep = 10;

	UPROPERTY(EditAnywhere, Category = "Auto Scaling")
		int32 MinScreenPercentage = 50;

	UPROPERTY(EditAnywhere, Category = "Auto Scaling")
		int32 MaxScreenPercentage = 100;

	void UpdateScreenPercentage();
};
