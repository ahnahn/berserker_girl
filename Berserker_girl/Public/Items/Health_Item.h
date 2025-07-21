// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Health_Item.generated.h"

/**
 * 
 */
UCLASS()
class BERSERKER_GIRL_API AHealth_Item : public AItem
{
	GENERATED_BODY()
protected:
	//virtual void BeginPlay() override;
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Health Properties")
		float Health;

	double DesiredZ;

	UPROPERTY(EditAnywhere)
		float DriftRate = 0.f;

public:
	virtual void Tick(float DeltaTime) override;
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float NumberOfHealth) { Health = NumberOfHealth; }
};
