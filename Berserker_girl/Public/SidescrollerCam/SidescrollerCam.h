// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnumDir.h"
#include "Components/TimelineComponent.h"
#include "SidescrollerCam.generated.h"

class UCapsuleComponent;
class USpringArmComponent;
class USceneComponent;
class UCineCameraComponent;
class ASwitchCamera;
class ACustomCamera;

UCLASS()
class BERSERKER_GIRL_API ASidescrollerCam : public AActor
{
	GENERATED_BODY()

public:
	ASidescrollerCam();

protected:
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

public:
	virtual void Tick(float DeltaTime) override;

	// =========================================================
	// Components
	// =========================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* Capsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		USceneComponent* CameraRigScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		USceneComponent* CameraMount;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		UCineCameraComponent* CineCamera;

	UFUNCTION(BlueprintPure, Category = "Camera")
		float GetCharacterDepthOffset() const { return CharacterDepthOffset; }


	//=========================================================================
	// Camera Settings
	//=========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "Set the length of the spring arm. This will adjust how far the camera is from the player."))
		float SpringArmLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "Adjust how high the camera is relative to the player"))
		float HeightOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "Adjust the pitch of the camera. This is useful when adjusting the Height Offset, as you can make sure to keep the player well framed."))
		float CameraPitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "This adjusts the focal length or zoom of the camera. Larger numbers are more zoomed-in."))
		float FocalLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "Adjust the aperture or f-stop of the camera. Smaller numbers(like 0.1) will give you a more shallow depth of field, and larger numbers will keep more of the scene in focus."))
		float Aperture;

	//=========================================================================
	// Camera Movement
	//=========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This adjusts the maximum distance that the camera will move ahead to show the player where they are going."))
		float MaxLeadDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This adjusts the maximum angle that the camera will turn to show the player where they are going."))
		float MaxLeadAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This adjusts how quickly the camera will adjust when the player moves."))
		float LeadSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This adjusts how quickly the camera will return to the player's position when the player stops moving."))
		float ReturnSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This allows you to select whether the camera should track your character when they run towards or away from the camera."))
		bool bTrackDepth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This allows you to set an offset amount that will make the camera track your character's depth movements more precisely(0) or more subtly(values closer to 1). If you set this to 1 the camera will not track the depth of the player."))
		float DepthTrackOffsetRatio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This adjusts how far the camera can lag behind when the player is jumping or falling. If you set this to a negative number, the camera will move ahead of the character when they fall. This can be useful to give the player a better view of where they will land."))
		float MaxHeightLagDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This adjusts how quickly the camera move to the Height Lag distance. Higher numbers move quicker."))
		float HeightLagSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This allows you to set how far the camera is allowed to zoom out when the player moves. This can help to show more of the level while the player is running."))
		float MaxZoomDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This controls how quickly the camera zooms in and out when the player moves."))
		float ZoomSpeed;

	//=========================================================================
	// Setup
	//=========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (ToolTip = "Place your Player Pawn in the world and Pick it with the dropper."))
		TObjectPtr<AActor> PlayerReference;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (ToolTip = "After picking your player character in the Player Reference box, click this box to connect the camera to your player. If you move the camera by mistake, you can click this box again to set it back to your player's location."))
		bool bConnectToPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (ToolTip = "Select the Right direction of your game. This is the direction your player runs if you press right on the keyboard or gamepad. This will automatically set up the entire system to use your game's orientation."))
		Enum_RightDir RightDirection = Enum_RightDir::X_Minus;

	UFUNCTION(BlueprintCallable, Category = "Camera")
		void SetFOV();

protected:

	UFUNCTION(BlueprintCallable, Category = "Camera Logic")
		void Fn_UpdateCamera();

	void Event_SwitchCamera();

	UFUNCTION()
		void OnCameraTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	FTimeline MoveTimeline;
	FTimeline SpringArmTimeline;

	UPROPERTY(EditAnywhere, Category = "Timeline")
		UCurveFloat* MoveCurve;

	UPROPERTY(EditAnywhere, Category = "Timeline")
		UCurveFloat* SpringArmCurve;

	UFUNCTION()
		void MoveTimelineUpdate(float Value);
	UFUNCTION()
		void SpringArmTimelineUpdate(float Value);
	UFUNCTION()
		void OnTransitionFinished();

	FTimerHandle PostTransitionDelayHandle;
	void OnPostTransitionDelayFinished();

	//=========================================================================
	// Internal State Variables
	//=========================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		FRotator SpringArmRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		bool bCameraIsCustom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		bool bCameraIsChanging;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		bool bCameraIsSwitchingToDefault;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float TargetFocalLength;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float DefaultFocusDistance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float DeltaSeconds;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float CurrentFocalLength;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float BlendTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		TObjectPtr<AActor> CurrentViewTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		FVector MovingFrom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float MoveTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		bool bIsMovingFromPlayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		FRotator RotatingFrom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float ReturnSpeedCache;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float LeadSpeedCache;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float CameraPitchCache;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float FocalLengthCache1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float FocalLengthCache2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		bool bIsTrackingX;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		bool bIsTrackingY;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		bool bIsTrackingZ;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		TObjectPtr<AActor> MovingFromActorRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		bool bSmoothCamera1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		bool bSmoothCamera2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		bool bCameraChangeInterupted;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float ApertureCache1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float ApertureCache2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float CurrentAperture;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		FVector StartLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float SpringArmTargetLength;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float CharacterDepthOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float DepthTrackingRatioSwitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float HeightOffsetSwitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float HeightOffsetSwitchNew;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float HeightOffsetSwitchOld;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float ScreenAspectFOVAdjust;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		float ZoomAdd;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State")
		TObjectPtr<ASwitchCamera> SwitchCameraRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Internal State", meta = (MultiLine = "true"))
		TObjectPtr<UCineCameraComponent> NewTargetCamera;

	// Timeline Components
	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera Timeline")
		//UTimelineComponent* MoveTimeline;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera Timeline")
		//UTimelineComponent* SpringArmTimeline;

};
