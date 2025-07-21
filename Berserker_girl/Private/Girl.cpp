// Fill out your copyright notice in the Description page of Project Settings.





#include "Girl.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "Components/AttributeComponent.h"

#include "Items/Item.h"

#include "Items/Weapons/Weapon.h"

#include "Components/StaticMeshComponent.h"

#include "Kismet/KismetMathLibrary.h"

#include "HUD/GirlHUD.h"

#include "HUD/GirlOverlay.h"

//#include "Items/Soul.h"

#include "Components/CapsuleComponent.h"

//#include "Items/Treasure.h"

#include "Items/Health_Item.h"



AGirl::AGirl()

{

	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;



	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

	GetMesh()->SetGenerateOverlapEvents(true);



	// Set Capsule Component Overlap

	//GetCapsuleComponent()->SetGenerateOverlapEvents(true); // This allows the component to trigger overlap events

	//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // This enables collision for the component

	//GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap); // This makes the component overlap with all channels

	bCanJumpAttack = true;



	//무적 초기화

	bIsInvincible = false;
	bWasOnGround = true; // 시작 시 땅에 있다고 가정
}



void AGirl::BeginPlay()

{

	Super::BeginPlay();



	Tags.Add(FName("EngageableTarget"));

	InitializeGirlOverlay();



	UWorld* World = GetWorld();

	if (World && WeaponClass)

	{

		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);

		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);

		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;

		EquippedWeapon = DefaultWeapon;

	}

}



void AGirl::Tick(float DeltaTime)

{

	if (Attributes && GirlOverlay)

	{

		Attributes->RegenStamina(DeltaTime);

		GirlOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());

	}
	//충돌 처리 (Tick 함수 내부)
	if (GetCharacterMovement())
	{
		bool bIsFalling = GetCharacterMovement()->IsFalling();

		if (bIsFalling && bWasOnGround) // 방금 땅에서 떨어졌을 때 (점프 포함)
		{
			if (GetCapsuleComponent())
			{
				// 다른 Pawn과의 충돌을 Overlap으로 변경
				GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
				UE_LOG(LogTemp, Warning, TEXT("Girl is now Falling - Pawn Collision: Overlap"));
			}
			bWasOnGround = false; // 공중에 있음
		}
		else if (!bIsFalling && !bWasOnGround) // 방금 땅에 착지했을 때
		{
			if (GetCapsuleComponent())
			{
				// 다른 Pawn과의 충돌을 다시 Block으로 변경
				GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
				UE_LOG(LogTemp, Warning, TEXT("Girl is now on Ground - Pawn Collision: Block"));
			}
			bWasOnGround = true; // 땅에 있음
		}
	}
}



void AGirl::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)

{

	Super::SetupPlayerInputComponent(PlayerInputComponent);



	PlayerInputComponent->BindAction(FName("Pad_A"), IE_Pressed, this, &AGirl::Keyboard_X_Gamepad_A_KeyPressed);

	PlayerInputComponent->BindAction(FName("Pad_A"), IE_Released, this, &AGirl::Keyboard_X_Gamepad_A_KeyReleased);

	PlayerInputComponent->BindAction(FName("Pad_B"), IE_Pressed, this, &AGirl::Keyboard_S_Gamepad_B_KeyPressed);

	PlayerInputComponent->BindAction(FName("Pad_X"), IE_Pressed, this, &AGirl::Keyboard_Z_Gamepad_X_KeyPressed);

	PlayerInputComponent->BindAction(FName("Pad_Y"), IE_Pressed, this, &AGirl::Keyboard_A_Gamepad_Y_KeyPressed);

	PlayerInputComponent->BindAction(FName("Interact"), IE_Pressed, this, &AGirl::Up_KeyPressed);



}





void AGirl::Keyboard_X_Gamepad_A_KeyPressed()

{

	if (IsOccupied() || !(Attributes && Attributes->GetStamina() > Attributes->GetJumpCost())) return;

	if (Attributes->GetHealthPercent() > 0.f)

	{

		if (ActionState == EActionState::EAS_Unoccupied)

		{

			Jump();

		}

	}

	ResetComboCounter();

	bCanJumpAttack = true;

	if (Attributes && GirlOverlay && !GetCharacterMovement()->IsFalling())

	{

		Attributes->UseStamina(Attributes->GetJumpCost());

		GirlOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());

	}

}



void AGirl::Keyboard_X_Gamepad_A_KeyReleased()

{

	StopJumping();

	ResetComboCounter();

}



void AGirl::Keyboard_S_Gamepad_B_KeyPressed()

{

	if (Attributes->GetHealthPercent() > 0.f)

	{

		if (GetCharacterMovement()->IsFalling() || IsOccupied() || !(Attributes && Attributes->GetStamina() > Attributes->GetDodgeCost())) return;

		// 구르기 시작 시 Pawn 충돌을 Overlap으로 변경
		bIsInvincible = true; // 무적 상태 시작
		if (GetCapsuleComponent())
		{
			GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
			UE_LOG(LogTemp, Warning, TEXT("Girl is now Dodging - Pawn Collision: Overlap"));
			GetWorld()->GetTimerManager().SetTimer(
				DodgeEffectTimerHandle,
				this,
				&AGirl::EndDodgeEffects,
				DodgeDuration // DodgeDuration 만큼만 무적/Overlap 유지
			);
		}

		PlayDodgeMontage();

		ActionState = EActionState::EAS_Dodge;

		if (Attributes && GirlOverlay)

		{

			Attributes->UseStamina(Attributes->GetDodgeCost());

			GirlOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());

		}

	}
}

void AGirl::Keyboard_Z_Gamepad_X_KeyPressed()

{

	Super::Attack();

	if (!(Attributes && Attributes->GetStamina() > Attributes->GetAttackCost()))

	{

		ResetComboCounter();

		return;

	}


	if (!GetCharacterMovement()->IsFalling() && CanAttack())

	{

		PlayAttackComboMontage();

		ActionState = EActionState::EAS_Attacking;

		if (Attributes && GirlOverlay)

		{

			Attributes->UseStamina(Attributes->GetAttackCost());

			GirlOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());

		}

	}

	else if (GetCharacterMovement()->IsFalling() && CanAttack() && bCanJumpAttack)

	{

		PlayJumpAttackMontage();

		ActionState = EActionState::EAS_Attacking;

		bCanJumpAttack = false;

		if (Attributes && GirlOverlay)

		{

			Attributes->UseStamina(Attributes->GetAttackCost());

			GirlOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());

		}

	}

}

void AGirl::Keyboard_A_Gamepad_Y_KeyPressed()

{

	if (IsOccupied()) return;

}



void AGirl::Up_KeyPressed()

{

}


float AGirl::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)

{

	// 무적 상태라면 데미지를 받지 않음

	if (bIsInvincible)

	{

		return 0.f;

	}

	HandleDamage(DamageAmount);

	SetHUDHealth();



	// 데미지를 입으면 무적 상태로 전환하고 타이머를 시작

	if (Attributes && Attributes->GetHealthPercent() > 0.f)

	{

		bIsInvincible = true;

		GetWorld()->GetTimerManager().SetTimer(

			InvincibilityTimerHandle,

			this,

			&AGirl::EndInvincibility,

			InvincibilityDuration

		);
		// 깜빡임 효과 시작
		GetWorld()->GetTimerManager().SetTimer(FlashTimerHandle, this, &AGirl::ToggleMeshVisibility, FlashInterval, true); // true: 반복
	}

	return DamageAmount;

}



void AGirl::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)

{

	// 무적 상태일 때는 피격 리액션을 스킵
	//if (bIsInvincible)
	//{
		//return;
	//}

	Super::GetHit_Implementation(ImpactPoint, Hitter);

	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);



	if (Attributes && Attributes->GetHealthPercent() > 0.f)

	{

		GetCharacterMovement()->Velocity = FVector::ZeroVector;

		ActionState = EActionState::EAS_HitReaction;

	}

	ResetComboCounter();

}



void AGirl::SetOverlappingItem(AItem* Item)

{

	OverlappingItem = Item;

}



void AGirl::AddHealth(AHealth_Item* Health_Item)

{

	if (Attributes && GirlOverlay)

	{

		Attributes->AddHealth(Health_Item->GetHealth());

		//GirlOverlay->SetHealth(Attributes->GetHealth());

	}

	SetHUDHealth();

}



//void AGirl::AddSouls(ASoul* Soul)

//{

//

// if (Attributes && GirlOverlay)

// {

// Attributes->AddSouls(Soul->GetSouls());

// GirlOverlay->SetSouls(Attributes->GetSouls());

// }

//}



//void AGirl::AddGold(ATreasure* Treasure)

//{

//

// if (Attributes && GirlOverlay)

// {

// Attributes->AddGold(Treasure->GetGold());

// GirlOverlay->SetGold(Attributes->GetGold());

// }

//}



void AGirl::HitReactEnd()

{

	ActionState = EActionState::EAS_Unoccupied;

}



void AGirl::InitializeGirlOverlay()

{

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)

	{

		AGirlHUD* GirlHUD = Cast<AGirlHUD>(PlayerController->GetHUD());

		if (GirlHUD)

		{

			GirlOverlay = GirlHUD->GetGirlOverlay();

			if (GirlOverlay && Attributes)

			{

				GirlOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());

				GirlOverlay->SetStaminaBarPercent(1.f);

				//GirlOverlay->SetGold(0);

				//GirlOverlay->SetSouls(0);

			}

		}

	}

}



void AGirl::SetHUDHealth()

{

	if (GirlOverlay && Attributes)

	{

		GirlOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());

	}

}



void AGirl::Die_Implementation()

{

	Super::Die_Implementation();



	ActionState = EActionState::EAS_Dead;

	DisableMeshCollision();



}



void AGirl::DodgeEnd()

{

	Super::DodgeEnd();

	ActionState = EActionState::EAS_Unoccupied;

	ResetComboCounter();
	// 구르기 애니메이션이 끝났을 때, 아직 효과가 활성화되어 있다면 강제로 종료
	// 타이머가 만료되기 전에 애니메이션이 끝나면 여기서 처리
	if (GetWorld()->GetTimerManager().IsTimerActive(DodgeEffectTimerHandle))
	{
		EndDodgeEffects();
	}
}



bool AGirl::CanAttack()

{

	return ActionState == EActionState::EAS_Unoccupied &&

		CharacterState != ECharacterState::ECS_Unequipped;

}



void AGirl::AttackEnd()

{

	ActionState = EActionState::EAS_Unoccupied;

}



bool AGirl::HasEnoughStamina()

{

	return Attributes && Attributes->GetStamina() > Attributes->GetDodgeCost();

}



bool AGirl::IsOccupied()

{

	return ActionState != EActionState::EAS_Unoccupied;

	//return ActionState != EActionState::EAS_Unoccupied || ActionState == EActionState::EAS_Dead;

}



bool AGirl::IsUnoccupied()

{

	return ActionState == EActionState::EAS_Unoccupied;

}



void AGirl::EndInvincibility()

{

	bIsInvincible = false;
	// 무적 시간이 끝나면 깜빡임 멈추기
	StopFlashing();
}
// 깜빡임 관련 함수 구현
void AGirl::ToggleMeshVisibility()
{
	if (GetMesh())
	{
		GetMesh()->SetVisibility(!GetMesh()->IsVisible()); // 현재 가시성을 반전
	}
}

void AGirl::StopFlashing()
{
	GetWorld()->GetTimerManager().ClearTimer(FlashTimerHandle); // 깜빡임 타이머 해제
	if (GetMesh())
	{
		GetMesh()->SetVisibility(true); // 메쉬를 항상 보이도록 설정 (깜빡임 중단 후)
	}
}
// 구르기 효과(무적/Overlap)를 해제
void AGirl::EndDodgeEffects()
{
	GetWorld()->GetTimerManager().ClearTimer(DodgeEffectTimerHandle); // 구르기 효과 타이머 해제
	//StopFlashing(); // 깜빡임도 구르기 효과 타이머와 함께 종료

	// 무적 상태는 다른 일반 피격 무적과 분리되므로, 구르기로 인한 무적만 해제
	// 일반 피격 무적은 InvincibilityTimerHandle에서 관리
	if (!GetWorld()->GetTimerManager().IsTimerActive(InvincibilityTimerHandle)) // 일반 피격 무적 타이머가 비활성화된 경우에만
	{
		bIsInvincible = false;
	}

	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
		UE_LOG(LogTemp, Warning, TEXT("Girl's Dodge Effects (Invincible/Overlap) Ended. Pawn Collision reset to Block."));
	}
}