// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwitchCamera.generated.h"

class USceneComponent;
class UTextRenderComponent;
class UBoxComponent;
class ACustomCamera;

UCLASS()
class BERSERKER_GIRL_API ASwitchCamera : public AActor
{
	GENERATED_BODY()

public:
	ASwitchCamera();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	//=========================================================
	// Components
	//=========================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UTextRenderComponent* TextRender;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* Box;

	//=========================================================================
	// Settings
	//=========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (MultiLine = "true", ToolTip = "Use the dropper to select the target camera(BP_CustomCamera) this blueprint should switch to. If it is left as None, the camera will switch back to the default camera."))
	TObjectPtr<ACustomCamera> NewViewTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (MultiLine = "true", ToolTip = "Set the time in seconds it should take to blend to the new camera."))
	float BlendTime;
};
