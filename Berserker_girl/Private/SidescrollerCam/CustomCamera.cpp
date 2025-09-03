// Fill out your copyright notice in the Description page of Project Settings.
#include "SidescrollerCam/CustomCamera.h"
#include "Components/SceneComponent.h"
#include "CineCameraComponent.h"
#include "SidescrollerCam/SidescrollerCam.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

ACustomCamera::ACustomCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	// =========================================================
	// Components
	// =========================================================
	
	// Scene Root
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// Cine Camera
	CineCamera = CreateDefaultSubobject<UCineCameraComponent>(TEXT("CineCamera"));
	CineCamera->SetupAttachment(SceneRoot);

	CineCamera->Filmback.SensorWidth = 23.76f;
	CineCamera->Filmback.SensorHeight = 13.365f;
	CineCamera->Filmback.SensorAspectRatio = 1.777778f;

	CineCamera->LensSettings.MinFocalLength = 4.0f;
	CineCamera->LensSettings.MaxFocalLength = 1000.0f;
	CineCamera->LensSettings.MinFStop = 0.01f;
	CineCamera->LensSettings.MaxFStop = 22.0f;
	CineCamera->LensSettings.DiaphragmBladeCount = 7;

	CineCamera->FocusSettings.FocusMethod = ECameraFocusMethod::Tracking;

	CineCamera->CurrentFocalLength = 35.0f;
	CineCamera->CurrentAperture = 2.8f;
	CineCamera->SetFieldOfView(37.497356f);
	CineCamera->AspectRatio = 1.777778f;

	// Camera Movement
	bTrackLeftRight = false;
	bTrackUpDown = false;
	bTrackDepth = false;
	LeftRightOffset = 0.0f;
	UpDownOffset = 0.0f;
	DepthOffset = 0.0f;
	bLeadCharacter = false;
	bSetCurrentTrackingOffsets = false;
	bSnapToTracking = false;
	DepthTrackOffsetRatio = 0.0f;

	// Camera Settings
	Aperture = 2.4f;
	FocusTargetActor = nullptr;
	FocalLength = 18.0f;

	// Setup
	bResetRotation = true;
	CameraRotationOffset = 180.0f;
	SidescrollerCam_Ref = nullptr;
	RightDirection = Enum_RightDir::Y_Plus;
}

void ACustomCamera::BeginPlay()
{
	Super::BeginPlay();

	if (!SidescrollerCam_Ref)
	{
		for (TActorIterator<ASidescrollerCam> It(GetWorld()); It; ++It)
		{
			SidescrollerCam_Ref = *It;
			break;
		}
	}
}

void ACustomCamera::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (SidescrollerCam_Ref)
	{
		switch (SidescrollerCam_Ref->RightDirection)
		{
		case Enum_RightDir::Y_Plus:
			CameraRotationOffset = 0.0f;
			break;
		case Enum_RightDir::Y_Minus:
			CameraRotationOffset = 180.0f;
			break;
		case Enum_RightDir::X_Plus:
			CameraRotationOffset = -90.0f;
			break;
		case Enum_RightDir::X_Minus:
			CameraRotationOffset = 90.0f;
			break;
		}

		if (bSetCurrentTrackingOffsets)
		{
			const FVector RelativeLocation = GetActorLocation() - SidescrollerCam_Ref->GetActorLocation();
			DepthOffset = RelativeLocation.X;
			LeftRightOffset = RelativeLocation.Y;
			UpDownOffset = RelativeLocation.Z;
			bSetCurrentTrackingOffsets = false;
		}

		if (bSnapToTracking)
		{
			Tick(0.0f);
			bSnapToTracking = false;
		}
	}

	if (bResetRotation)
	{
		SceneRoot->SetWorldRotation(FRotator(0.0f, CameraRotationOffset, 0.0f));
		bResetRotation = false;
	}

	if (CineCamera)
	{
		CineCamera->CurrentAperture = Aperture;
		CineCamera->CurrentFocalLength = FocalLength;

		AActor* TargetToFocus = FocusTargetActor;
		if (!TargetToFocus)
		{
			TargetToFocus = SidescrollerCam_Ref;
		}

		if (FocusTargetActor)
		{
			CineCamera->FocusSettings.FocusMethod = ECameraFocusMethod::Tracking;
			CineCamera->FocusSettings.TrackingFocusSettings.ActorToTrack = FocusTargetActor;
		}
		else
		{
			CineCamera->FocusSettings.FocusMethod = ECameraFocusMethod::Manual;
			CineCamera->FocusSettings.TrackingFocusSettings.ActorToTrack = nullptr;
		}
	}
}

void ACustomCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!SidescrollerCam_Ref || !(bTrackLeftRight || bTrackUpDown || bTrackDepth))
	{
		return;
	}

	UCapsuleComponent* Capsule = SidescrollerCam_Ref->FindComponentByClass<UCapsuleComponent>();
	if (!Capsule)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = Capsule->GetComponentLocation();

	switch (RightDirection)
	{
	case Enum_RightDir::Y_Plus:
	case Enum_RightDir::Y_Minus:
	{
			const float TargetX = TargetLocation.X + DepthOffset + (SidescrollerCam_Ref->GetCharacterDepthOffset() * DepthTrackOffsetRatio);
			const float TargetY = TargetLocation.Y + LeftRightOffset;
			const float TargetZ = TargetLocation.Z + UpDownOffset;

			const float NewX = bTrackDepth ? TargetX : CurrentLocation.X;
			const float NewY = bTrackLeftRight ? TargetY : CurrentLocation.Y;
			const float NewZ = bTrackUpDown ? TargetZ : CurrentLocation.Z;

			SetActorLocation(FVector(NewX, NewY, NewZ));
			break;
	}

	case Enum_RightDir::X_Plus:
	case Enum_RightDir::X_Minus:
	{
		const float TargetX = TargetLocation.X + LeftRightOffset;
		const float TargetY = TargetLocation.Y + DepthOffset + (SidescrollerCam_Ref->GetCharacterDepthOffset() * DepthTrackOffsetRatio);
		const float TargetZ = TargetLocation.Z + UpDownOffset;

		const float NewX = bTrackLeftRight ? TargetX : CurrentLocation.X;
		const float NewY = bTrackDepth ? TargetY : CurrentLocation.Y;
		const float NewZ = bTrackUpDown ? TargetZ : CurrentLocation.Z;

		SetActorLocation(FVector(NewX, NewY, NewZ));
		break;
	}
	}
}
