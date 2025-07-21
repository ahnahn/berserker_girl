// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/GirlHUD.h"
#include "HUD/GirlOverlay.h"

void AGirlHUD::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* Controller = World->GetFirstPlayerController();
		if (Controller && GirlOverlayClass)
		{
			GirlOverlay = CreateWidget<UGirlOverlay>(Controller, GirlOverlayClass);
			GirlOverlay->AddToViewport();
		}
	}
}