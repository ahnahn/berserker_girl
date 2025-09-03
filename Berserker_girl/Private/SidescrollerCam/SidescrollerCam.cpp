// Fill out your copyright notice in the Description page of Project Settings.

#include "SidescrollerCam/SidescrollerCam.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "CineCameraComponent.h"
#include "SidescrollerCam/SwitchCamera.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "SidescrollerCam/CustomCamera.h"
#include "TimerManager.h"

ASidescrollerCam::ASidescrollerCam()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root (Capsule)
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(34.f, 88.f);
	RootComponent = Capsule;

	// Camera Rig Scene (child of Root)
	CameraRigScene = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRigScene"));
	CameraRigScene->SetupAttachment(RootComponent);

	// SpringArm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CameraRigScene);
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->SetUsingAbsoluteRotation(true);

	// Camera Mount
	CameraMount = CreateDefaultSubobject<USceneComponent>(TEXT("CameraMount"));
	CameraMount->SetupAttachment(SpringArm);

	// Cine Camera
	CineCamera = CreateDefaultSubobject<UCineCameraComponent>(TEXT("CineCamera"));
	CineCamera->SetupAttachment(CameraMount);

	// CineCamera Default Values
	CineCamera->Filmback.SensorWidth = 23.76f;
	CineCamera->Filmback.SensorHeight = 13.365f;
	CineCamera->FocusSettings.FocusMethod = ECameraFocusMethod::Tracking;
	CineCamera->CurrentFocalLength = 15.0f;
	CineCamera->CurrentAperture = 2.8f;
	CineCamera->bConstrainAspectRatio = false;

	//=========================================================================
	// Camera Settings Defaults
	//=========================================================================
	SpringArmLength = 845.0f;
	HeightOffset = 150.0f;
	CameraPitch = -10.0f;
	FocalLength = 15.0f;
	Aperture = 2.0f;

	//=========================================================================
	// Camera Movement Defaults
	//=========================================================================
	MaxLeadDistance = 250.0f;
	MaxLeadAngle = 2.0f;
	LeadSpeed = 1.0f;
	ReturnSpeed = 0.5f;
	bTrackDepth = true;
	DepthTrackOffsetRatio = 0.0f;
	MaxHeightLagDistance = 250.0f;
	HeightLagSpeed = 1.0f;
	MaxZoomDistance = 0.0f;
	ZoomSpeed = 1.0f;

	//=========================================================================
	// Setup Defaults
	//=========================================================================
	PlayerReference = nullptr;
	bConnectToPlayer = false;
	RightDirection = Enum_RightDir::X_Minus;

	//=========================================================================
	// Internal State Variable Defaults
	//=========================================================================
	bCameraIsCustom = false;
	bCameraIsChanging = false;
	bCameraIsSwitchingToDefault = false;
	CurrentViewTarget = nullptr;
	bIsMovingFromPlayer = true;
	ReturnSpeedCache = 0.5f;
	LeadSpeedCache = 1.0f;
	CameraPitchCache = -10.0f;
	FocalLengthCache1 = 15.0f;
	FocalLengthCache2 = 15.0f;
	bIsTrackingX = true;
	bIsTrackingY = true;
	bIsTrackingZ = true;
	bSmoothCamera1 = true;
	bSmoothCamera2 = true;
	bCameraChangeInterupted = false;
	ApertureCache1 = 2.0f;
	ApertureCache2 = 2.0f;
	DepthTrackingRatioSwitch = 1.0f;
	HeightOffsetSwitch = 1.0f;
	HeightOffsetSwitchNew = 1.0f;
	HeightOffsetSwitchOld = 0.0f;
	NewTargetCamera = nullptr;
}

void ASidescrollerCam::BeginPlay()
{
	Super::BeginPlay();

	// Bind the overlap event
	if (Capsule)
	{
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &ASidescrollerCam::OnCameraTriggerOverlap);
	}

	// Setup Timelines
	if (MoveCurve)
	{
		FOnTimelineFloat MoveTimelineUpdateDelegate;
		MoveTimelineUpdateDelegate.BindUFunction(this, FName("MoveTimelineUpdate"));
		MoveTimeline.AddInterpFloat(MoveCurve, MoveTimelineUpdateDelegate);

		FOnTimelineEvent MoveTimelineFinishedDelegate;
		MoveTimelineFinishedDelegate.BindUFunction(this, FName("OnTransitionFinished"));
		MoveTimeline.SetTimelineFinishedFunc(MoveTimelineFinishedDelegate);
	}

	if (SpringArmCurve)
	{
		FOnTimelineFloat SpringArmTimelineUpdateDelegate;
		SpringArmTimelineUpdateDelegate.BindUFunction(this, FName("SpringArmTimelineUpdate"));
		SpringArmTimeline.AddInterpFloat(SpringArmCurve, SpringArmTimelineUpdateDelegate);
	}

	SetFOV();

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController)
	{
		PlayerController->SetViewTargetWithBlend(this, 0.0f);
	}

	CurrentViewTarget = this;

	if (PlayerReference)
	{
		StartLocation = PlayerReference->GetActorLocation();
	}

	SpringArmTargetLength = SpringArmLength;
	HeightOffsetSwitchOld = 0.0f;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACustomCamera::StaticClass(), FoundActors);
	for (AActor* Actor : FoundActors)
	{
		ACustomCamera* CustomCam = Cast<ACustomCamera>(Actor);
		if (CustomCam)
		{
			CustomCam->SidescrollerCam_Ref = this;
			CustomCam->RightDirection = this->RightDirection;
		}
	}
}

void ASidescrollerCam::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Tick timelines
	MoveTimeline.TickTimeline(DeltaTime);
	SpringArmTimeline.TickTimeline(DeltaTime);

	DeltaSeconds = DeltaTime;
	Fn_UpdateCamera();

	if (PlayerReference)
	{
		const FVector PlayerLocation = PlayerReference->GetActorLocation();
		float CalculatedDepthOffset = 0.0f;

		switch (RightDirection)
		{
		case Enum_RightDir::Y_Plus: CalculatedDepthOffset = (StartLocation.X - PlayerLocation.X) * -1.f; break;
		case Enum_RightDir::Y_Minus: CalculatedDepthOffset = StartLocation.X - PlayerLocation.X; break;
		case Enum_RightDir::X_Plus: CalculatedDepthOffset = StartLocation.Y - PlayerLocation.Y; break;
		case Enum_RightDir::X_Minus: CalculatedDepthOffset = (StartLocation.Y - PlayerLocation.Y) * -1.f; break;
		}

		const bool bIsMoving = FMath::Abs(CalculatedDepthOffset) > 200.f;
		const float InterpSpeed = bIsMoving ? LeadSpeed : ReturnSpeed;
		CharacterDepthOffset = UKismetMathLibrary::Lerp(CharacterDepthOffset, CalculatedDepthOffset, DeltaSeconds * InterpSpeed);
	}

	if (SpringArm && CineCamera)
	{
		const float FinalArmLength = SpringArmTargetLength + (ZoomAdd * CharacterDepthOffset * DepthTrackingRatioSwitch);
		SpringArm->TargetArmLength = FinalArmLength;
		const float FinalFocalLength = TargetFocalLength + (ZoomAdd * ScreenAspectFOVAdjust);
		CineCamera->CurrentFocalLength = FinalFocalLength;
	}
}

void ASidescrollerCam::OnCameraTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASwitchCamera* SwitchCamera = Cast<ASwitchCamera>(OtherActor);
	if (!SwitchCamera)
	{
		return;
	}

	if (SwitchCameraRef == SwitchCamera)
	{
		return;
	}

	MovingFromActorRef = CurrentViewTarget;
	SwitchCameraRef = SwitchCamera;
	MoveTime = SwitchCameraRef->BlendTime;

	ACustomCamera* CustomCamTarget = SwitchCameraRef->NewViewTarget;

	bCameraIsSwitchingToDefault = !IsValid(CustomCamTarget);

	if (bCameraIsSwitchingToDefault)
	{
		NewTargetCamera = this->CineCamera;
	}
	else
	{
		NewTargetCamera = CustomCamTarget->CineCamera;
	}

	if (bCameraIsCustom || bCameraIsChanging || !bCameraIsSwitchingToDefault)
	{
		// Set CurrentViewTarget
		if (IsValid(CustomCamTarget))
		{
			CurrentViewTarget = CustomCamTarget;
		}
		else
		{
			CurrentViewTarget = this;
		}

		ACustomCamera* CastedCustomCam = Cast<ACustomCamera>(CurrentViewTarget);
		if (CastedCustomCam) // Cast Succeeded
		{
			bIsTrackingY = CastedCustomCam->bTrackLeftRight;
			bIsTrackingZ = CastedCustomCam->bTrackUpDown;
			HeightOffsetSwitchNew = UKismetMathLibrary::Conv_BoolToFloat(CastedCustomCam->bTrackUpDown);
			bIsTrackingX = bTrackDepth;
			bSmoothCamera1 = CastedCustomCam->bLeadCharacter;
			bSmoothCamera2 = CastedCustomCam->bLeadCharacter;
		}
		else // Cast Failed
		{
			bIsTrackingY = true;
			bSmoothCamera2 = true;
			bIsTrackingZ = true;
			bIsTrackingX = bTrackDepth;
		}
	}

	Event_SwitchCamera();
}

void ASidescrollerCam::Event_SwitchCamera()
{

	if (CameraRigScene && CameraRigScene->GetAttachParent())
	{
		CameraRigScene->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	bCameraChangeInterupted = bCameraIsChanging;

	// Cache current settings
	ApertureCache1 = CineCamera->CurrentAperture;
	FocalLengthCache1 = TargetFocalLength;
	HeightOffsetSwitchOld = HeightOffsetSwitch;
	ReturnSpeedCache = ReturnSpeed;
	LeadSpeedCache = LeadSpeed;
	CameraPitchCache = CameraPitch;

	ApertureCache2 = bCameraIsSwitchingToDefault ? this->Aperture : NewTargetCamera->CurrentAperture;
	FocalLengthCache2 = bCameraIsSwitchingToDefault ? this->FocalLength : NewTargetCamera->CurrentFocalLength;

	// Set initial transform caches for interpolation
	if (bCameraChangeInterupted)
	{
		MovingFrom = CameraRigScene->GetComponentLocation();
		RotatingFrom = SpringArm->GetComponentRotation();
	}
	else
	{
		MovingFrom = bIsMovingFromPlayer ? this->GetActorLocation() : MovingFromActorRef->GetActorLocation();

		if (bIsMovingFromPlayer)
		{
			RotatingFrom = SpringArmRotation;
		}
		else
		{
			// Check if MovingFromActorRef is a CustomCamera to decide rotation source
			ACustomCamera* CustomCamRef = Cast<ACustomCamera>(MovingFromActorRef);
			RotatingFrom = (CustomCamRef) ? CustomCamRef->GetActorRotation() : MovingFromActorRef->GetActorRotation();
		}
	}

	bCameraIsChanging = true;

	// Set timeline play rates based on blend time
	float NewPlayRate = UKismetMathLibrary::SafeDivide(1.0f, MoveTime);
	if (NewPlayRate > 0)
	{
		MoveTimeline.SetPlayRate(NewPlayRate);
		SpringArmTimeline.SetPlayRate(NewPlayRate);
	}

	// Start timelines
	if (bCameraIsSwitchingToDefault)
	{
		SpringArmTimeline.Reverse();
	}
	else
	{
		SpringArmTimeline.Play();
	}

	MoveTimeline.PlayFromStart();
}

void ASidescrollerCam::MoveTimelineUpdate(float Value)
{
	// Lerp World Location and Rotation for the camera rig
	FVector TargetLocation = IsValid(CurrentViewTarget) ? CurrentViewTarget->GetActorLocation() : this->GetActorLocation();
	FVector NewLocation = FMath::Lerp(MovingFrom, TargetLocation, Value);
	CameraRigScene->SetWorldLocation(NewLocation);

	FRotator TargetRotation;
	if (bIsMovingFromPlayer)
	{
		TargetRotation = SpringArmRotation;
	}
	else
	{
		ACustomCamera* CustomCamRef = Cast<ACustomCamera>(MovingFromActorRef);
		TargetRotation = IsValid(CustomCamRef) ? CustomCamRef->GetActorRotation() : MovingFromActorRef->GetActorRotation();
	}

	FRotator NewRotation = FMath::Lerp(RotatingFrom, TargetRotation, Value);
	SpringArm->SetWorldRotation(NewRotation);

	// Lerp Camera Settings
	CurrentFocalLength = FMath::Lerp(FocalLengthCache1, FocalLengthCache2, Value);
	TargetFocalLength = CurrentFocalLength;

	CurrentAperture = FMath::Lerp(ApertureCache1, ApertureCache2, Value);
	if (CineCamera)
	{
		CineCamera->CurrentAperture = CurrentAperture;
	}
}

void ASidescrollerCam::SpringArmTimelineUpdate(float Value)
{
	DepthTrackingRatioSwitch = FMath::Lerp(1.0f, UKismetMathLibrary::SelectFloat(DepthTrackOffsetRatio, 2.0f, bSmoothCamera1), Value);
	SpringArmTargetLength = FMath::Lerp(SpringArmLength, 0.0f, Value);

	float TargetHeightOffset = FMath::Lerp(HeightOffset, 0.0f, Value);
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, TargetHeightOffset));

	HeightOffsetSwitch = FMath::Lerp(HeightOffsetSwitchOld, HeightOffsetSwitchNew, Value);
}

void ASidescrollerCam::OnTransitionFinished()
{
	bIsMovingFromPlayer = false;

	// After the move is finished, decide whether to attach or detach the camera rig
	if (bCameraIsSwitchingToDefault)
	{
		// Attach back to self (Player camera)
		CameraRigScene->AttachToComponent(this->RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		bSmoothCamera1 = true;
	}
	else
	{
		// Attach to the new custom camera's root
		if (IsValid(CurrentViewTarget))
		{
			CameraRigScene->AttachToComponent(CurrentViewTarget->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
		}
		bSmoothCamera1 = true;
	}

	// Start a delay before setting bCameraIsChanging to false
	GetWorld()->GetTimerManager().SetTimer(PostTransitionDelayHandle, this, &ASidescrollerCam::OnPostTransitionDelayFinished, 0.2f, false);
}

void ASidescrollerCam::OnPostTransitionDelayFinished()
{
	bCameraIsChanging = false;
	bCameraIsCustom = !bCameraIsSwitchingToDefault;
}

void ASidescrollerCam::Fn_UpdateCamera()
{
	if (!PlayerReference || !CineCamera) return;

	bool bShouldZeroVelocity = ((bCameraIsCustom || bCameraIsChanging) && !bCameraIsSwitchingToDefault && !bIsTrackingY) || !bSmoothCamera2;
	FVector InputVelocity = bShouldZeroVelocity ? FVector::ZeroVector : PlayerReference->GetVelocity();

	const FRotator CurrentCameraRotation = CineCamera->GetRelativeRotation();
	const FVector CurrentCameraLocation = CineCamera->GetRelativeLocation();

	float LeadVelocity = 0.0f;
	float DepthVelocity = 0.0f;
	float CurrentLeadLocation = 0.0f;
	float CurrentDepthLocation = 0.0f;

	switch (RightDirection)
	{
	case Enum_RightDir::Y_Plus:
	case Enum_RightDir::Y_Minus:
		LeadVelocity = InputVelocity.Y;
		DepthVelocity = InputVelocity.X;
		CurrentLeadLocation = CurrentCameraLocation.Y;
		CurrentDepthLocation = CurrentCameraLocation.X;
		break;
	case Enum_RightDir::X_Plus:
	case Enum_RightDir::X_Minus:
		LeadVelocity = InputVelocity.X;
		DepthVelocity = InputVelocity.Y;
		CurrentLeadLocation = CurrentCameraLocation.X;
		CurrentDepthLocation = CurrentCameraLocation.Y;
		break;
	}

	float TargetYaw = (RightDirection == Enum_RightDir::Y_Minus || RightDirection == Enum_RightDir::X_Minus)
		? UKismetMathLibrary::MapRangeClamped(LeadVelocity, -600.f, 600.f, MaxLeadAngle, -MaxLeadAngle)
		: UKismetMathLibrary::MapRangeClamped(LeadVelocity, -600.f, 600.f, -MaxLeadAngle, MaxLeadAngle);

	const bool bIsMovingHorizontally = FMath::Abs(LeadVelocity) > 200.f;
	const float YawInterpSpeed = bIsMovingHorizontally ? (LeadSpeed * 2.0f) : (ReturnSpeed * 2.0f);
	const float NewYaw = UKismetMathLibrary::Lerp(CurrentCameraRotation.Yaw, TargetYaw, DeltaSeconds * YawInterpSpeed);

	float TargetLeadLocation = (RightDirection == Enum_RightDir::Y_Minus || RightDirection == Enum_RightDir::X_Minus)
		? UKismetMathLibrary::MapRangeClamped(LeadVelocity, -600.f, 600.f, MaxLeadDistance, -MaxLeadDistance)
		: UKismetMathLibrary::MapRangeClamped(LeadVelocity, -600.f, 600.f, -MaxLeadDistance, MaxLeadDistance);

	const float LeadInterpSpeed = bIsMovingHorizontally ? LeadSpeed : ReturnSpeed;
	const float NewLeadLocation = UKismetMathLibrary::Lerp(CurrentLeadLocation, TargetLeadLocation, DeltaSeconds * LeadInterpSpeed);

	float TargetDepthLocation = (RightDirection == Enum_RightDir::Y_Minus || RightDirection == Enum_RightDir::X_Minus)
		? UKismetMathLibrary::MapRangeClamped(DepthVelocity, -600.f, 600.f, MaxLeadDistance, -MaxLeadDistance)
		: UKismetMathLibrary::MapRangeClamped(DepthVelocity, -600.f, 600.f, -MaxLeadDistance, MaxLeadDistance);

	const bool bIsMovingVertically = FMath::Abs(DepthVelocity) > 200.f;
	const float DepthInterpSpeed = bIsMovingVertically ? LeadSpeed : ReturnSpeed;
	const bool bShouldTrackDepth = (RightDirection == Enum_RightDir::Y_Plus || RightDirection == Enum_RightDir::Y_Minus) ? bIsTrackingX : bIsTrackingY;
	const float FinalTargetDepthLocation = bShouldTrackDepth ? TargetDepthLocation : 0.f;
	const float NewDepthLocation = UKismetMathLibrary::Lerp(CurrentDepthLocation, FinalTargetDepthLocation, DeltaSeconds * DepthInterpSpeed);

	const float TargetHeightLocation = UKismetMathLibrary::MapRangeClamped(InputVelocity.Z, -1200.f, 1200.f, -MaxHeightLagDistance, MaxHeightLagDistance);
	const float FinalTargetHeight = bIsTrackingZ ? TargetHeightLocation : 0.f;
	const float HeightLagInterpSpeed = UKismetMathLibrary::MapRangeClamped(InputVelocity.Z, 500.f, -900.f, 1.f, HeightLagSpeed);
	const float NewHeightLocation = UKismetMathLibrary::Lerp(CurrentCameraLocation.Z, FinalTargetHeight, DeltaSeconds * HeightLagSpeed * HeightLagInterpSpeed);

	CineCamera->SetRelativeRotation(FRotator(CameraPitch, NewYaw, 0.f));

	FVector NewLocation;
	if (RightDirection == Enum_RightDir::Y_Plus || RightDirection == Enum_RightDir::Y_Minus)
	{
		NewLocation.X = NewDepthLocation * DepthTrackingRatioSwitch;
		NewLocation.Y = NewLeadLocation;
	}
	else
	{
		NewLocation.X = NewLeadLocation;
		NewLocation.Y = NewDepthLocation * DepthTrackingRatioSwitch;
	}
	NewLocation.Z = NewHeightLocation * HeightOffsetSwitch;
	CineCamera->SetRelativeLocation(NewLocation);

	const float AbsLeadVelocity = FMath::Abs(LeadVelocity);
	const float TargetZoomAdd = UKismetMathLibrary::MapRangeClamped(AbsLeadVelocity, 0.f, 600.f, 0.f, MaxZoomDistance);
	const float ZoomInterpSpeed = UKismetMathLibrary::MapRangeClamped(AbsLeadVelocity, 0.f, 600.f, ReturnSpeed, ZoomSpeed);
	ZoomAdd = UKismetMathLibrary::Lerp(ZoomAdd, TargetZoomAdd, DeltaSeconds * ZoomInterpSpeed);
}

void ASidescrollerCam::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!CineCamera || !SpringArm) return;

	FCameraFocusSettings NewFocusSettings;
	NewFocusSettings.FocusMethod = ECameraFocusMethod::Tracking;
	FCameraTrackingFocusSettings TrackingSettings;
	TrackingSettings.ActorToTrack = this;
	NewFocusSettings.TrackingFocusSettings = TrackingSettings;
	CineCamera->FocusSettings = NewFocusSettings;

	if (RightDirection == Enum_RightDir::X_Plus || RightDirection == Enum_RightDir::X_Minus)
	{
		bIsTrackingY = bTrackDepth;
	}
	else
	{
		bIsTrackingX = bTrackDepth;
	}

	CineCamera->CurrentAperture = Aperture;
	CineCamera->CurrentFocalLength = FocalLength;
	TargetFocalLength = FocalLength;
	CineCamera->SetRelativeRotation(FRotator(CameraPitch, 0.0f, 0.0f));

	SpringArm->TargetArmLength = SpringArmLength;
	SpringArm->CameraLagSpeed = 30.0f;
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, HeightOffset));

	FRotator NewSpringArmRotation;
	switch (RightDirection)
	{
	case Enum_RightDir::Y_Plus: NewSpringArmRotation = FRotator(0.f, 0.f, 0.f); break;
	case Enum_RightDir::Y_Minus: NewSpringArmRotation = FRotator(0.f, 180.f, 0.f); break;
	case Enum_RightDir::X_Plus: NewSpringArmRotation = FRotator(0.f, 270.f, 0.f); break;
	case Enum_RightDir::X_Minus: NewSpringArmRotation = FRotator(0.f, 90.f, 0.f); break;
	default: NewSpringArmRotation = FRotator::ZeroRotator; break;
	}
	SpringArm->SetRelativeRotation(NewSpringArmRotation);
	SpringArmRotation = NewSpringArmRotation;

	if (bConnectToPlayer)
	{
		if (IsValid(PlayerReference))
		{
			const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false);
			AttachToActor(PlayerReference, AttachmentRules);
		}
		bConnectToPlayer = false;
	}
}

void ASidescrollerCam::SetFOV()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController) return;

	int32 ViewportSizeX, ViewportSizeY;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

	if (ViewportSizeY == 0) return;

	const double AspectRatio = static_cast<double>(ViewportSizeX) / static_cast<double>(ViewportSizeY);
	const double InRangeA = 1.77777;
	const double InRangeB = 2.4;
	const double OutRangeA = 1.0;
	const double OutRangeB = (1.0 / 97.0) * 78.0;

	ScreenAspectFOVAdjust = UKismetMathLibrary::MapRangeClamped(AspectRatio, InRangeA, InRangeB, OutRangeA, OutRangeB);
}
