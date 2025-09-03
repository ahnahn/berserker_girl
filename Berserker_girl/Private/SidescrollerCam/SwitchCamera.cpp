// Fill out your copyright notice in the Description page of Project Settings.
#include "SidescrollerCam/SwitchCamera.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/BoxComponent.h"
#include "UObject/ConstructorHelpers.h"

ASwitchCamera::ASwitchCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	//=========================================================
	// Component Creation & Setup
	//=========================================================

	// Default Scene Root
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;
	DefaultSceneRoot->bVisualizeComponent = true;

	// Text Render Component
	TextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRender"));
	TextRender->SetupAttachment(RootComponent);
	TextRender->SetText(FText::FromString(TEXT("Switch Cam")));
	TextRender->SetHorizontalAlignment(EHTA_Center);
	TextRender->SetVerticalAlignment(EVRTA_TextCenter);
	TextRender->SetRelativeLocation(FVector(-0.000031f, 0.000000f, 51.651768f));
	TextRender->bHiddenInGame = true;

	TextRender->SetWorldScale3D(FVector(2.0f, 2.0f, 2.0f));


	// Box Component
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComponent);
	Box->SetBoxExtent(FVector(50.000000f, 100.000000f, 100.000000f));

	//=========================================================================
	// Settings Default Values
	//=========================================================================

	NewViewTarget = nullptr;
	BlendTime = 2.0f;
}

void ASwitchCamera::BeginPlay()
{
	Super::BeginPlay();

}

void ASwitchCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
