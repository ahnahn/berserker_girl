// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnumDir.h"
#include "CustomCamera.generated.h"

class ASidescrollerCam;
class UCineCameraComponent;
class USceneComponent;

UCLASS()
class BERSERKER_GIRL_API ACustomCamera : public AActor
{
	GENERATED_BODY()

public:
	ACustomCamera();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	virtual void Tick(float DeltaTime) override;

	// =========================================================
	// Components
	// =========================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
		UCineCameraComponent* CineCamera;

	//=========================================================================
	// Camera Movement Variables
	//=========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This selects whether the camera should move left and right with the player, or stay in the left-right position it was placed in the world."))
		bool bTrackLeftRight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This selects whether the camera should move up and down with the player, or stay in the Up-Down position it was placed in the world."))
		bool bTrackUpDown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This selects whether the camera should track your character when they run towards or away from the camera, or stay at the depth it was placed in the world."))
		bool bTrackDepth = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This adjust how far left or right of the player the camera will be when tracking the player. This is only used when \"Track Left Right\" is checked."))
		float LeftRightOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This adjust how far above or below the player the camera will be when tracking the player. This is only used when \"Track Up Down\" is checked."))
		float UpDownOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This adjust how far away from the player the camera will be when tracking the player. This is only used when \"Track Depth\" is checked."))
		float DepthOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This selects whether the camera should move ahead of the player when the player is moving. This is only used if \"Track Left Right\" is checked."))
		bool bLeadCharacter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This sets the current location relative to the player as the 3 tracking offsets."))
		bool bSetCurrentTrackingOffsets = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This snaps the camera to the location relative to the player determined by the offsets."))
		bool bSnapToTracking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement", meta = (ToolTip = "This allows you to set an offset amount that will make the camera track your character's depth movements more precisely(0) or more subtly(values closer to 1). If you set this to 1 the camera will not track the depth of the player."))
		float DepthTrackOffsetRatio = 0.0f;

	//=========================================================================
	// Camera Settings Variables
	//=========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "Adjust the aperture or f-stop of the camera. Smaller numbers will give you a more shallow depth of field, and larger numbers will keep more of the scene in focus."))
		float Aperture = 2.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "Select the actor that the camera will focus on. This becomes more apparent when using smaller Aperture values."))
		TObjectPtr<AActor> FocusTargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings", meta = (ToolTip = "This adjusts the focal length or zoom of the camera. Larger numbers are more zoomed-in."))
		float FocalLength = 18.0f;

	//=========================================================================
	// Setup Variables
	//=========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (ToolTip = "This resets the rotation of the camera to the default depending on the Right-Direction of your game."))
		bool bResetRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (ToolTip = "An additional offset applied to the camera's rotation."))
		float CameraRotationOffset = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (ToolTip = "Reference to the main SidescrollerCam actor that manages this camera."))
		TObjectPtr<ASidescrollerCam> SidescrollerCam_Ref;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup", meta = (ToolTip = "Defines the 'Right' direction for the game world, affecting camera orientation."))
		Enum_RightDir RightDirection = Enum_RightDir::Y_Plus;
};
