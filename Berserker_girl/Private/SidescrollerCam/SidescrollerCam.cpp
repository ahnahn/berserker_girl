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

// 이 클래스는 게임의 메인 카메라 액터로, 플레이어를 따라다니며 시점을 제어하는 모든 핵심 로직을 담당
ASidescrollerCam::ASidescrollerCam()
{
	// 매 프레임 카메라 위치를 부드럽게 갱신하기 위해 Tick을 활성화
	PrimaryActorTick.bCanEverTick = true;

	// Root (Capsule)
	// 이 액터의 루트 컴포넌트로 캡슐을 사용. SwitchCamera와의 오버랩 이벤트를 감지하는 역할
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(34.f, 88.f);
	RootComponent = Capsule;

	// Camera Rig Scene (child of Root)
	// 카메라의 모든 구성요소(스프링암, 카메라 등)를 담는 컨테이너 역할을 하는 SceneComponent
	// 이 컴포넌트의 위치를 조절하여 전체 카메라 리그를 움직임
	CameraRigScene = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRigScene"));
	CameraRigScene->SetupAttachment(RootComponent);

	// SpringArm
	// 카메라와 플레이어 사이의 거리를 유지하고, 부드러운 지연(Lag) 효과를 주는 컴포넌트
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(CameraRigScene);
	SpringArm->bDoCollisionTest = false; // 횡스크롤 게임에서는 카메라가 벽 뒤로 숨는 경우가 거의 없으므로 충돌 테스트를 비활성화
	SpringArm->bEnableCameraLag = true; // 카메라의 움직임에 부드러운 지연 효과를 추가
	SpringArm->SetUsingAbsoluteRotation(true); // 스프링암이 부모의 회전값에 영향을 받지 않고 월드 기준의 절대 회전값을 사용하게 함

	// Camera Mount
	// 카메라를 최종적으로 부착할 지점
	//추가적인 위치나 회전 오프셋을 적용할 때 유용
	CameraMount = CreateDefaultSubobject<USceneComponent>(TEXT("CameraMount"));
	CameraMount->SetupAttachment(SpringArm);

	// Cine Camera
	// 실제 화면을 렌더링하는 시네마틱 카메라 컴포넌트
	CineCamera = CreateDefaultSubobject<UCineCameraComponent>(TEXT("CineCamera"));
	CineCamera->SetupAttachment(CameraMount);

	// CineCamera Default Values
	// 시네 카메라 기본 설정값
	CineCamera->Filmback.SensorWidth = 23.76f;
	CineCamera->Filmback.SensorHeight = 13.365f;
	CineCamera->FocusSettings.FocusMethod = ECameraFocusMethod::Tracking;
	CineCamera->CurrentFocalLength = 15.0f;
	CineCamera->CurrentAperture = 2.8f;
	CineCamera->bConstrainAspectRatio = false;

	//=========================================================================
	// Camera Settings Defaults
	//=========================================================================
	
	// 에디터에서 조절 가능한 카메라의 기본 설정값들을 초기화
	SpringArmLength = 845.0f;
	HeightOffset = 150.0f;
	CameraPitch = -10.0f;
	FocalLength = 15.0f;
	Aperture = 2.0f;

	//=========================================================================
	// Camera Movement Defaults
	//=========================================================================
	
	// 카메라의 동적인 움직임(리드, 지연, 줌 등)을 제어하는 변수들을 초기화
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
	
	// 카메라 시스템의 내부 상태를 관리하는 변수들을 초기화
	bCameraIsCustom = false; // 현재 커스텀 카메라 시점인지 여부
	bCameraIsChanging = false; // 현재 카메라 시점이 전환 중인지 여부
	bCameraIsSwitchingToDefault = false; // 기본 카메라로 복귀 중인지 여부
	CurrentViewTarget = nullptr; // 현재 시점의 목표 액터
	bIsMovingFromPlayer = true;
	// 카메라 전환 시 부드러운 보간을 위해 이전 설정값을 저장하는 캐시 변수들
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

// 게임 시작 시 호출되는 함수
void ASidescrollerCam::BeginPlay()
{
	Super::BeginPlay();

	// 캡슐 컴포넌트의 OnComponentBeginOverlap 델리게이트에 OnCameraTriggerOverlap 함수를 바인딩
	// 이를 통해 이 액터가 다른 액터와 오버랩될 때마다 해당 함수가 자동으로 호출
	if (Capsule)
	{
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &ASidescrollerCam::OnCameraTriggerOverlap);
	}

	// 카메라 전환 시 부드러운 움직임을 구현하기 위한 타임라인들을 설정
	if (MoveCurve)
	{
		FOnTimelineFloat MoveTimelineUpdateDelegate;
		// 타임라인이 업데이트될 때마다 MoveTimelineUpdate 함수를 호출하도록 델리게이트를 바인딩
		MoveTimelineUpdateDelegate.BindUFunction(this, FName("MoveTimelineUpdate"));
		MoveTimeline.AddInterpFloat(MoveCurve, MoveTimelineUpdateDelegate);

		FOnTimelineEvent MoveTimelineFinishedDelegate;
		// 타임라인 재생이 끝나면 OnTransitionFinished 함수를 호출하도록 델리게이트를 바인딩
		MoveTimelineFinishedDelegate.BindUFunction(this, FName("OnTransitionFinished"));
		MoveTimeline.SetTimelineFinishedFunc(MoveTimelineFinishedDelegate);
	}

	if (SpringArmCurve)
	{
		FOnTimelineFloat SpringArmTimelineUpdateDelegate;
		SpringArmTimelineUpdateDelegate.BindUFunction(this, FName("SpringArmTimelineUpdate"));
		SpringArmTimeline.AddInterpFloat(SpringArmCurve, SpringArmTimelineUpdateDelegate);
	}

	// 화면 비율에 맞춰 FOV 값을 설정
	SetFOV();

	// 게임 시작 시 플레이어의 시점을 이 카메라 액터로 설정
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

	// 월드에 배치된 모든 CustomCamera 액터를 찾아, 이 메인 카메라에 대한 참조를 설정
	// 이를 통해 각 CustomCamera가 메인 카메라의 상태를 참조할 수 있게 됨
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

// 매 프레임 호출되는 함수
void ASidescrollerCam::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Tick timelines
	// 매 프레임 타임라인을 수동으로 업데이트
	MoveTimeline.TickTimeline(DeltaTime);
	SpringArmTimeline.TickTimeline(DeltaTime);

	DeltaSeconds = DeltaTime;
	// 플레이어 추적 및 카메라 리드 효과 등 핵심 카메라 로직을 매 프레임 실행
	Fn_UpdateCamera();

	if (PlayerReference)
	{
		// 플레이어의 Z축(게임 내 깊이) 이동에 따라 카메라 오프셋을 계산
		const FVector PlayerLocation = PlayerReference->GetActorLocation();
		float CalculatedDepthOffset = 0.0f;

		switch (RightDirection)
		{
		case Enum_RightDir::Y_Plus: CalculatedDepthOffset = (StartLocation.X - PlayerLocation.X) * -1.f; break;
		case Enum_RightDir::Y_Minus: CalculatedDepthOffset = StartLocation.X - PlayerLocation.X; break;
		case Enum_RightDir::X_Plus: CalculatedDepthOffset = StartLocation.Y - PlayerLocation.Y; break;
		case Enum_RightDir::X_Minus: CalculatedDepthOffset = (StartLocation.Y - PlayerLocation.Y) * -1.f; break;
		}

		// Lerp를 사용하여 깊이 오프셋 값을 부드럽게 보간
		const bool bIsMoving = FMath::Abs(CalculatedDepthOffset) > 200.f;
		const float InterpSpeed = bIsMoving ? LeadSpeed : ReturnSpeed;
		CharacterDepthOffset = UKismetMathLibrary::Lerp(CharacterDepthOffset, CalculatedDepthOffset, DeltaSeconds * InterpSpeed);
	}

	if (SpringArm && CineCamera)
	{
		// 최종 계산된 값들을 스프링암과 카메라에 적용
		const float FinalArmLength = SpringArmTargetLength + (ZoomAdd * CharacterDepthOffset * DepthTrackingRatioSwitch);
		SpringArm->TargetArmLength = FinalArmLength;
		const float FinalFocalLength = TargetFocalLength + (ZoomAdd * ScreenAspectFOVAdjust);
		CineCamera->CurrentFocalLength = FinalFocalLength;
	}
}

// 플레이어에 부착된 메인 카메라의 캡슐이 다른 액터와 오버랩될 때 호출되는 이벤트 함수
// 카메라 전환 시스템의 시작점 역할
void ASidescrollerCam::OnCameraTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 1. 오버랩된 액터가 카메라 전환 트리거(ASwitchCamera)인지 Cast를 통해 확인
	ASwitchCamera* SwitchCamera = Cast<ASwitchCamera>(OtherActor);
	if (!SwitchCamera) // 카메라 트리거가 아니라면 아무것도 하지 않고 즉시 함수를 종료
	{
		return;
	}

	// 2. 이미 현재 활성화된 트리거에 다시 진입한 경우, 불필요한 중복 실행을 방지
	if (SwitchCameraRef == SwitchCamera)
	{
		return;
	}

	// 3. 전환에 필요한 정보들을 트리거로부터 가져와 변수에 캐싱
	MovingFromActorRef = CurrentViewTarget;     // 현재 시점의 액터 (전환 시작점)
	SwitchCameraRef = SwitchCamera;             // 새로 진입한 트리거
	MoveTime = SwitchCameraRef->BlendTime;      // 트리거에 설정된 전환(블렌딩) 시간

	// 4. 전환할 목표 카메라(CustomCamera) 가져오기
	ACustomCamera* CustomCamTarget = SwitchCameraRef->NewViewTarget;

	// 5. 목표 카메라가 유효한지(지정되어 있는지)에 따라 '특수 카메라로 전환'할지
	// '기본 카메라로 복귀'할지를 결정
	// 목표 카메라가 nullptr이면 기본 카메라로 복귀하는 것으로 간주
	bCameraIsSwitchingToDefault = !IsValid(CustomCamTarget);

	if (bCameraIsSwitchingToDefault)
	{
		NewTargetCamera = this->CineCamera; // 목표를 자기 자신(기본 카메라)으로 설정
	}
	else
	{
		// 목표를 트리거에 지정된 커스텀 카메라로 설정
		NewTargetCamera = CustomCamTarget->CineCamera;
	}

	// 목표 카메라의 추적 설정 등을 가져오는 로직
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
	// 6. 모든 사전 준비가 끝나면, 타임라인을 설정하고 재생하는 실제 카메라 전환 이벤트 함수를 호출
	Event_SwitchCamera();
}

// 실제 카메라 전환 로직을 담당하는 함수
//타임라인 재생을 준비하고 시작
void ASidescrollerCam::Event_SwitchCamera()
{
	// 전환 시작 시, 카메라 리그를 현재 부모로부터 분리하여 월드 공간에 독립적으로 만듦
	if (CameraRigScene && CameraRigScene->GetAttachParent())
	{
		CameraRigScene->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	// 이전 전환이 끝나기 전에 새로운 전환이 시작되었는지 확인
	bCameraChangeInterupted = bCameraIsChanging;

	// 부드러운 보간(Lerp)을 위해, 전환 시작 시점의 카메라 설정값들을 캐시 변수에 저장
	ApertureCache1 = CineCamera->CurrentAperture;
	FocalLengthCache1 = TargetFocalLength;
	HeightOffsetSwitchOld = HeightOffsetSwitch;
	ReturnSpeedCache = ReturnSpeed;
	LeadSpeedCache = LeadSpeed;
	CameraPitchCache = CameraPitch;

	// 전환 목표 카메라의 설정값들을 캐시 변수에 저장
	ApertureCache2 = bCameraIsSwitchingToDefault ? this->Aperture : NewTargetCamera->CurrentAperture;
	FocalLengthCache2 = bCameraIsSwitchingToDefault ? this->FocalLength : NewTargetCamera->CurrentFocalLength;

	// 보간의 시작점(위치, 회전)을 설정
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
			ACustomCamera* CustomCamRef = Cast<ACustomCamera>(MovingFromActorRef);
			RotatingFrom = (CustomCamRef) ? CustomCamRef->GetActorRotation() : MovingFromActorRef->GetActorRotation();
		}
	}

	// 현재 카메라가 '전환 중' 상태
	bCameraIsChanging = true;

	// 트리거에 설정된 전환 시간(MoveTime)에 맞춰 타임라인의 재생 속도를 동적으로 조절
	float NewPlayRate = UKismetMathLibrary::SafeDivide(1.0f, MoveTime);
	if (NewPlayRate > 0)
	{
		MoveTimeline.SetPlayRate(NewPlayRate);
		SpringArmTimeline.SetPlayRate(NewPlayRate);
	}

	// 최종적으로 타임라인을 재생하여 부드러운 전환을 시작
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

// 타임라인이 재생되는 동안 매 틱 호출되어, 카메라 리그의 위치와 회전을 보간하는 함수
void ASidescrollerCam::MoveTimelineUpdate(float Value)
{
	// Value는 타임라인의 진행도(0.0 ~ 1.0)를 나타냄
	// FMath::Lerp를 사용하여 시작 위치/회전과 목표 위치/회전 사이의 중간값을 계산하고 적용
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
	// 초점 거리, 조리개 등 카메라의 세부 세팅 또한 부드럽게 보간하여 적용
	CurrentFocalLength = FMath::Lerp(FocalLengthCache1, FocalLengthCache2, Value);
	TargetFocalLength = CurrentFocalLength;

	CurrentAperture = FMath::Lerp(ApertureCache1, ApertureCache2, Value);
	if (CineCamera)
	{
		CineCamera->CurrentAperture = CurrentAperture;
	}
}

// 타임라인이 재생되는 동안 매 틱 호출되어, 스프링암 관련 값들을 보간하는 함수
void ASidescrollerCam::SpringArmTimelineUpdate(float Value)
{
	DepthTrackingRatioSwitch = FMath::Lerp(1.0f, UKismetMathLibrary::SelectFloat(DepthTrackOffsetRatio, 2.0f, bSmoothCamera1), Value);
	SpringArmTargetLength = FMath::Lerp(SpringArmLength, 0.0f, Value);

	float TargetHeightOffset = FMath::Lerp(HeightOffset, 0.0f, Value);
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, TargetHeightOffset));

	HeightOffsetSwitch = FMath::Lerp(HeightOffsetSwitchOld, HeightOffsetSwitchNew, Value);
}

// 카메라 전환 타임라인 재생이 모두 끝났을 때 호출되는 함수
void ASidescrollerCam::OnTransitionFinished()
{
	bIsMovingFromPlayer = false;

	// 전환이 끝난 후, 카메라 리그를 최종 목표 액터에 부착하여 다음 움직임을 준비
	if (bCameraIsSwitchingToDefault)
	{
		// 기본 카메라로 복귀하는 경우, 카메라 리그를 다시 자기 자신(메인 카메라)에게 부착
		CameraRigScene->AttachToComponent(this->RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		bSmoothCamera1 = true;
	}
	else
	{
		// 커스텀 카메라로 전환하는 경우, 해당 커스텀 카메라 액터에게 부착
		if (IsValid(CurrentViewTarget))
		{
			CameraRigScene->AttachToComponent(CurrentViewTarget->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
		}
		bSmoothCamera1 = true;
	}

	// 전환 직후 바로 또 다른 전환이 일어나는 것을 방지하기 위해 짧은 딜레이를 줌
	GetWorld()->GetTimerManager().SetTimer(PostTransitionDelayHandle, this, &ASidescrollerCam::OnPostTransitionDelayFinished, 0.2f, false);
}

// 전환 후 딜레이가 끝났을 때 호출되는 함수
void ASidescrollerCam::OnPostTransitionDelayFinished()
{
	bCameraIsChanging = false;
	bCameraIsCustom = !bCameraIsSwitchingToDefault;
}

// 매 프레임 호출되어 기본 카메라의 움직임(리드, 랙, 줌 등)을 처리하는 함수
void ASidescrollerCam::Fn_UpdateCamera()
{
	if (!PlayerReference || !CineCamera) return;

	// 현재 커스텀 카메라 뷰 상태 등, 플레이어를 따라가지 않아야 할 조건을 확인
	bool bShouldZeroVelocity = ((bCameraIsCustom || bCameraIsChanging) && !bCameraIsSwitchingToDefault && !bIsTrackingY) || !bSmoothCamera2;
	FVector InputVelocity = bShouldZeroVelocity ? FVector::ZeroVector : PlayerReference->GetVelocity();

	const FRotator CurrentCameraRotation = CineCamera->GetRelativeRotation();
	const FVector CurrentCameraLocation = CineCamera->GetRelativeLocation();

	// 플레이어의 속도를 X, Y, Z축별로 분리하여 계산 준비
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

	// 플레이어의 좌우 이동 속도에 따라 카메라의 Yaw 회전값을 계산하여, 플레이어의 이동 방향을 미리 보여주는 '리드' 효과를 줌
	float TargetYaw = (RightDirection == Enum_RightDir::Y_Minus || RightDirection == Enum_RightDir::X_Minus)
		? UKismetMathLibrary::MapRangeClamped(LeadVelocity, -600.f, 600.f, MaxLeadAngle, -MaxLeadAngle)
		: UKismetMathLibrary::MapRangeClamped(LeadVelocity, -600.f, 600.f, -MaxLeadAngle, MaxLeadAngle);

	// Lerp를 사용하여 현재 Yaw 값에서 목표 Yaw 값으로 부드럽게 회전
	const bool bIsMovingHorizontally = FMath::Abs(LeadVelocity) > 200.f;
	const float YawInterpSpeed = bIsMovingHorizontally ? (LeadSpeed * 2.0f) : (ReturnSpeed * 2.0f);
	const float NewYaw = UKismetMathLibrary::Lerp(CurrentCameraRotation.Yaw, TargetYaw, DeltaSeconds * YawInterpSpeed);

	// 플레이어의 이동 방향으로 카메라를 먼저 보내는 '리드' 위치를 계산
	float TargetLeadLocation = (RightDirection == Enum_RightDir::Y_Minus || RightDirection == Enum_RightDir::X_Minus)
		? UKismetMathLibrary::MapRangeClamped(LeadVelocity, -600.f, 600.f, MaxLeadDistance, -MaxLeadDistance)
		: UKismetMathLibrary::MapRangeClamped(LeadVelocity, -600.f, 600.f, -MaxLeadDistance, MaxLeadDistance);

	// 현재 카메라 위치에서 목표 리드 위치로 부드럽게 이동
	const float LeadInterpSpeed = bIsMovingHorizontally ? LeadSpeed : ReturnSpeed;
	const float NewLeadLocation = UKismetMathLibrary::Lerp(CurrentLeadLocation, TargetLeadLocation, DeltaSeconds * LeadInterpSpeed);

	// 깊이(Depth)와 높이(Height)에 대해서도 동일한 방식으로 부드러운 지연(Lag) 효과를 계산
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

	// 최종 계산된 회전값과 위치값을 카메라에 적용
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

	// 플레이어의 이동 속도에 따라 미세한 줌 효과를 적용
	const float AbsLeadVelocity = FMath::Abs(LeadVelocity);
	const float TargetZoomAdd = UKismetMathLibrary::MapRangeClamped(AbsLeadVelocity, 0.f, 600.f, 0.f, MaxZoomDistance);
	const float ZoomInterpSpeed = UKismetMathLibrary::MapRangeClamped(AbsLeadVelocity, 0.f, 600.f, ReturnSpeed, ZoomSpeed);
	ZoomAdd = UKismetMathLibrary::Lerp(ZoomAdd, TargetZoomAdd, DeltaSeconds * ZoomInterpSpeed);
}

// 에디터에서 값이 변경될 때마다 호출되는 함수
void ASidescrollerCam::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!CineCamera || !SpringArm) return;

	// 포커스 대상을 자기 자신으로 설정
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

	// 에디터에서 조정한 값들을 즉시 카메라와 스프링암에 반영
	CineCamera->CurrentAperture = Aperture;
	CineCamera->CurrentFocalLength = FocalLength;
	TargetFocalLength = FocalLength;
	CineCamera->SetRelativeRotation(FRotator(CameraPitch, 0.0f, 0.0f));

	SpringArm->TargetArmLength = SpringArmLength;
	SpringArm->CameraLagSpeed = 30.0f;
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, HeightOffset));

	// 횡스크롤 방향에 맞춰 스프링암의 기본 회전값을 설정
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

	// 에디터에서 bConnectToPlayer를 체크하면 즉시 플레이어에게 이 카메라를 부착
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

// 화면의 가로세로 비율(Aspect Ratio)에 맞춰 FOV(화각)를 보정하는 함수
void ASidescrollerCam::SetFOV()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController) return;

	int32 ViewportSizeX, ViewportSizeY;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

	if (ViewportSizeY == 0) return;

	// 현재 화면 비율을 계산
	const double AspectRatio = static_cast<double>(ViewportSizeX) / static_cast<double>(ViewportSizeY);
	const double InRangeA = 1.77777; // 16:9
	const double InRangeB = 2.4;     // 와이드스크린
	const double OutRangeA = 1.0;
	const double OutRangeB = (1.0 / 97.0) * 78.0;

	// MapRangeClamped를 사용하여 화면 비율에 따라 FOV 보정값을 계산
	ScreenAspectFOVAdjust = UKismetMathLibrary::MapRangeClamped(AspectRatio, InRangeA, InRangeB, OutRangeA, OutRangeB);
}
