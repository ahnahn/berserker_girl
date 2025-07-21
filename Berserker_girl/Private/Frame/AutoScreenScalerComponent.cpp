#include "Frame/AutoScreenScalerComponent.h"
#include "Engine/Engine.h"

UAutoScreenScalerComponent::UAutoScreenScalerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;
}

void UAutoScreenScalerComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentScreenPercentage = MaxScreenPercentage;
	FString Command = FString::Printf(TEXT("r.ScreenPercentage %d"), CurrentScreenPercentage);
	GEngine->Exec(GetWorld(), *Command);
}

void UAutoScreenScalerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	float CurrentFPS = 1.0f / DeltaTime;
	UE_LOG(LogTemp, Warning, TEXT("FPS: %.1f | DeltaTime: %.5f"), CurrentFPS, DeltaTime);
	AccumulatedTime += DeltaTime;
	FrameCount++;
	TimeSinceLastAdjust += DeltaTime;

	if (AccumulatedTime >= FPSCheckIntervalSeconds && TimeSinceLastAdjust >= MinAdjustmentInterval)
	{
		AverageFPS = FrameCount / AccumulatedTime;
		UE_LOG(LogTemp, Warning, TEXT("AutoScaler Tick (from FApp): FPS=%.1f"), AverageFPS);

		UpdateScreenPercentage();

		AccumulatedTime = 0.f;
		FrameCount = 0;
		TimeSinceLastAdjust = 0.f;
	}
}

void UAutoScreenScalerComponent::UpdateScreenPercentage()
{
	UE_LOG(LogTemp, Warning, TEXT("AutoScaler Tick: AvgFPS=%.1f, CurrPct=%d"), AverageFPS, CurrentScreenPercentage);


	bool bChanged = false;

	if (AverageFPS < CriticalLowFPS && CurrentScreenPercentage > MinScreenPercentage)
	{
		// 급격한 하락
		CurrentScreenPercentage = FMath::Max(CurrentScreenPercentage - AggressiveDropStep, MinScreenPercentage);
		bChanged = true;
	}
	else if (AverageFPS < TargetFPS && CurrentScreenPercentage > MinScreenPercentage)
	{
		// 천천히 하락
		CurrentScreenPercentage = FMath::Max(CurrentScreenPercentage - 1, MinScreenPercentage);
		bChanged = true;
	}
	else if (AverageFPS >= TargetFPS && CurrentScreenPercentage < MaxScreenPercentage)
	{
		// 천천히 회복
		CurrentScreenPercentage = FMath::Min(CurrentScreenPercentage + 1, MaxScreenPercentage);
		bChanged = true;
	}
	// TargetFPS ~ TargetFPS+Margin 구간에서는 변화 없음

	if (bChanged)
	{
		FString Command = FString::Printf(TEXT("r.ScreenPercentage %d"), CurrentScreenPercentage);
		GEngine->Exec(GetWorld(), *Command);
		UE_LOG(LogTemp, Warning, TEXT("r.ScreenPercentage adjusted to: %d"), CurrentScreenPercentage);
	}
}
