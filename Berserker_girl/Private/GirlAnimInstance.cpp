// Fill out your copyright notice in the Description page of Project Settings.


#include "GirlAnimInstance.h"
#include "Girl.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UGirlAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Girl = Cast<AGirl>(TryGetPawnOwner());
	if (Girl)
	{
		GirlMovement = Girl->GetCharacterMovement();
	}
}

void UGirlAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (GirlMovement)
	{
		FVector Velocity = Girl->GetVelocity();
		Velocity.Z = 0.f;
		Speed = Velocity.Size();

		bIsInAir = Girl->GetCharacterMovement()->IsFalling();
		bIsAccelerating = Girl->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
		CharacterState = Girl->GetCharacterState();
		ActionState = Girl->GetActionState();
		DeathPose = Girl->GetDeathPose();
	}

}