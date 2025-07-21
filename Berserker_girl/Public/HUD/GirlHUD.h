// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GirlHUD.generated.h"

/**
 * 
 */
class UGirlOverlay;

UCLASS()
class BERSERKER_GIRL_API AGirlHUD : public AHUD
{
	GENERATED_BODY()
	
protected:
	virtual void PreInitializeComponents() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = Girl)
		TSubclassOf<UGirlOverlay> GirlOverlayClass;

	UPROPERTY()
		UGirlOverlay* GirlOverlay;
public:
	FORCEINLINE UGirlOverlay* GetGirlOverlay() const { return GirlOverlay; }
};