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



	//���� �ʱ�ȭ

	bIsInvincible = false;
	bWasOnGround = true; // ���� �� ���� �ִٰ� ����
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
	//�浹 ó�� (Tick �Լ� ����)
	if (GetCharacterMovement())
	{
		bool bIsFalling = GetCharacterMovement()->IsFalling();

		if (bIsFalling && bWasOnGround) // ��� ������ �������� �� (���� ����)
		{
			if (GetCapsuleComponent())
			{
				// �ٸ� Pawn���� �浹�� Overlap���� ����
				GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
				UE_LOG(LogTemp, Warning, TEXT("Girl is now Falling - Pawn Collision: Overlap"));
			}
			bWasOnGround = false; // ���߿� ����
		}
		else if (!bIsFalling && !bWasOnGround) // ��� ���� �������� ��
		{
			if (GetCapsuleComponent())
			{
				// �ٸ� Pawn���� �浹�� �ٽ� Block���� ����
				GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
				UE_LOG(LogTemp, Warning, TEXT("Girl is now on Ground - Pawn Collision: Block"));
			}
			bWasOnGround = true; // ���� ����
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

		// ������ ���� �� Pawn �浹�� Overlap���� ����
		bIsInvincible = true; // ���� ���� ����
		if (GetCapsuleComponent())
		{
			GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
			UE_LOG(LogTemp, Warning, TEXT("Girl is now Dodging - Pawn Collision: Overlap"));
			GetWorld()->GetTimerManager().SetTimer(
				DodgeEffectTimerHandle,
				this,
				&AGirl::EndDodgeEffects,
				DodgeDuration // DodgeDuration ��ŭ�� ����/Overlap ����
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

	// ���� ���¶�� �������� ���� ����

	if (bIsInvincible)

	{

		return 0.f;

	}

	HandleDamage(DamageAmount);

	SetHUDHealth();



	// �������� ������ ���� ���·� ��ȯ�ϰ� Ÿ�̸Ӹ� ����

	if (Attributes && Attributes->GetHealthPercent() > 0.f)

	{

		bIsInvincible = true;

		GetWorld()->GetTimerManager().SetTimer(

			InvincibilityTimerHandle,

			this,

			&AGirl::EndInvincibility,

			InvincibilityDuration

		);
		// ������ ȿ�� ����
		GetWorld()->GetTimerManager().SetTimer(FlashTimerHandle, this, &AGirl::ToggleMeshVisibility, FlashInterval, true); // true: �ݺ�
	}

	return DamageAmount;

}



void AGirl::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)

{

	// ���� ������ ���� �ǰ� ���׼��� ��ŵ
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
	// ������ �ִϸ��̼��� ������ ��, ���� ȿ���� Ȱ��ȭ�Ǿ� �ִٸ� ������ ����
	// Ÿ�̸Ӱ� ����Ǳ� ���� �ִϸ��̼��� ������ ���⼭ ó��
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
	// ���� �ð��� ������ ������ ���߱�
	StopFlashing();
}
// ������ ���� �Լ� ����
void AGirl::ToggleMeshVisibility()
{
	if (GetMesh())
	{
		GetMesh()->SetVisibility(!GetMesh()->IsVisible()); // ���� ���ü��� ����
	}
}

void AGirl::StopFlashing()
{
	GetWorld()->GetTimerManager().ClearTimer(FlashTimerHandle); // ������ Ÿ�̸� ����
	if (GetMesh())
	{
		GetMesh()->SetVisibility(true); // �޽��� �׻� ���̵��� ���� (������ �ߴ� ��)
	}
}
// ������ ȿ��(����/Overlap)�� ����
void AGirl::EndDodgeEffects()
{
	GetWorld()->GetTimerManager().ClearTimer(DodgeEffectTimerHandle); // ������ ȿ�� Ÿ�̸� ����
	//StopFlashing(); // �����ӵ� ������ ȿ�� Ÿ�̸ӿ� �Բ� ����

	// ���� ���´� �ٸ� �Ϲ� �ǰ� ������ �и��ǹǷ�, ������� ���� ������ ����
	// �Ϲ� �ǰ� ������ InvincibilityTimerHandle���� ����
	if (!GetWorld()->GetTimerManager().IsTimerActive(InvincibilityTimerHandle)) // �Ϲ� �ǰ� ���� Ÿ�̸Ӱ� ��Ȱ��ȭ�� ��쿡��
	{
		bIsInvincible = false;
	}

	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
		UE_LOG(LogTemp, Warning, TEXT("Girl's Dodge Effects (Invincible/Overlap) Ended. Pawn Collision reset to Block."));
	}
}