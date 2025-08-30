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
#include "Components/CapsuleComponent.h"
#include "Items/Health_Item.h"
#include "Kismet/GameplayStatics.h"
//#include "Items/Soul.h"
//#include "Items/Treasure.h"

AGirl::AGirl()
{
	// 매 프레임 Tick 함수를 호출
	PrimaryActorTick.bCanEverTick = true;

	// 캐릭터가 컨트롤러의 회전 대신 이동 방향을 바라보게 설정
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// 캐릭터 메시에 대한 충돌 설정을 정의
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	// 점프 공격 가능 여부 플래그
	bCanJumpAttack = true;

	// 무적 관련 변수 초기화
	bIsInvincible = false; // 피격 후 무적
	bIsDodgeInvincible = false; // 회피(Dodge) 중 무적
	bWasOnGround = true; // 시작 시 땅에 있다고 가정
}

void AGirl::BeginPlay()
{
	Super::BeginPlay();

	// 적 캐릭터가 주인공 캐릭터를 공격 대상으로 인식할 수 있도록 태그를 추가
	Tags.Add(FName("EngageableTarget"));

	// HUD UI와 캐릭터의 데이터를 연동하는 초기화 함수를 호출
	InitializeGirlOverlay();

	// 게임 시작 시 기본 무기를 생성하고 장착
	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		EquippedWeapon = DefaultWeapon;
	}
}

// 매 프레임 호출되는 함수
void AGirl::Tick(float DeltaTime)
{
	// 스태미나를 매 프레임 자연 회복시키고, HUD에 반영
	if (Attributes && GirlOverlay)
	{
		Attributes->RegenStamina(DeltaTime);
		GirlOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}

	// 충돌 처리 (Tick 함수 내부)
	// [동적 충돌 제어]
	// 점프나 낙하 시 다른 캐릭터와 겹칠 수 있도록 충돌 설정을 동적으로 변경
	// 이를 통해 공중에서 다른 캐릭터와 부딪혀 막히는 경험을 방지
	if (GetCharacterMovement())
	{
		bool bIsFalling = GetCharacterMovement()->IsFalling();

		// Case 1: 땅에 있다가 공중으로 떠오른 순간 (점프, 낙하)
		if (bIsFalling && bWasOnGround) // 방금 땅에서 떨어졌을 때 (점프 포함)
		{
			if (GetCapsuleComponent())
			{
				// 다른 Pawn과의 충돌을 Overlap으로 변경
				// Pawn 채널과의 충돌을 Block에서 Overlap으로 변경
				GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
				//(LogTemp, Warning, TEXT("Girl is now Falling - Pawn Collision: Overlap"));
			}
			bWasOnGround = false; // 공중에 있음
		}
		// Case 2: 공중에 있다가 땅에 착지한 순간
		else if (!bIsFalling && !bWasOnGround) // 방금 땅에 착지했을 때
		{
			if (GetCapsuleComponent())
			{
				// Pawn 채널과의 충돌을 다시 Block으로 복원
				GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
				//UE_LOG(LogTemp, Warning, TEXT("Girl is now on Ground - Pawn Collision: Block"));
			}
			bWasOnGround = true; // 땅에 있음
		}
	}
}

// 플레이어의 입력을 함수와 바인딩(연결)하는 함수
void AGirl::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 키보드와 게임패드 입력을 각 액션 함수에 연결
	PlayerInputComponent->BindAction(FName("Pad_A"), IE_Pressed, this, &AGirl::Keyboard_X_Gamepad_A_KeyPressed);
	PlayerInputComponent->BindAction(FName("Pad_A"), IE_Released, this, &AGirl::Keyboard_X_Gamepad_A_KeyReleased);
	PlayerInputComponent->BindAction(FName("Pad_B"), IE_Pressed, this, &AGirl::Keyboard_S_Gamepad_B_KeyPressed);
	PlayerInputComponent->BindAction(FName("Pad_X"), IE_Pressed, this, &AGirl::Keyboard_Z_Gamepad_X_KeyPressed);
	PlayerInputComponent->BindAction(FName("Pad_Y"), IE_Pressed, this, &AGirl::Keyboard_A_Gamepad_Y_KeyPressed);
	PlayerInputComponent->BindAction(FName("Interact"), IE_Pressed, this, &AGirl::Up_KeyPressed);
	PlayerInputComponent->BindAction(FName("Pad_Start"), IE_Pressed, this, &AGirl::PauseGame);
	PlayerInputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AGirl::PauseGame);
}

// 점프 입력 처리 함수
void AGirl::Keyboard_X_Gamepad_A_KeyPressed()
{
	// IsOccupied()를 통해 다른 행동 중에는 점프할 수 없도록 막음
	// 스태미나가 부족할 경우에도 점프할 수 없음
	if (IsOccupied() || !(Attributes && Attributes->GetStamina() > Attributes->GetJumpCost()))
		return;

	if (Attributes->GetHealthPercent() > 0.f)
	{
		// Unoccupied 상태일 때만 점프가 가능
		if (ActionState == EActionState::EAS_Unoccupied)
		{
			Jump();
		}
	}

	// 점프 시 콤보 카운터를 리셋하고, 점프 공격이 가능하도록 플래그를 설정
	ResetComboCounter();
	bCanJumpAttack = true;

	// 지상에서의 점프일 경우에만 스태미나를 소모
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

// 회피(Dodge) 입력 처리 함수
void AGirl::Keyboard_S_Gamepad_B_KeyPressed()
{
	if (Attributes->GetHealthPercent() > 0.f)
	{
		// 공중, 다른 행동 중, 스태미나 부족 시에는 회피할 수 없음
		if (GetCharacterMovement()->IsFalling() || IsOccupied() || !(Attributes && Attributes->GetStamina() > Attributes->GetDodgeCost()))
			return;

		// 1. 회피 시작 시 '회피 무적' 상태(bIsDodgeInvincible)만 true로 설정
		// 이는 '피격 후 무적'(bIsInvincible)과 완벽히 분리하기 위함
		bIsDodgeInvincible = true;
		if (GetCapsuleComponent())
		{
			// 2. Pawn 채널과의 충돌을 Overlap으로 변경하여 적을 통과할 수 있도록 함
			GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

			// 3. FTimerManager를 사용하여 일정 시간(DodgeDuration) 후에 회피 효과(무적 및 충돌)를
			// 해제하는 함수(EndDodgeEffects)를 예약
			GetWorld()->GetTimerManager().SetTimer(
				DodgeEffectTimerHandle,
				this,
				&AGirl::EndDodgeEffects,
				DodgeDuration // DodgeDuration(0.5초) 만큼만 회피 무적/Overlap 유지
			);
		}

		PlayDodgeMontage();
		// 4. 캐릭터의 상태를 '회피 중'으로 명확히 하여 다른 행동이 불가능하도록 제어
		ActionState = EActionState::EAS_Dodge;

		if (Attributes && GirlOverlay)
		{
			Attributes->UseStamina(Attributes->GetDodgeCost());
			GirlOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
		}
	}
}

// 공격 입력 처리 함수
void AGirl::Keyboard_Z_Gamepad_X_KeyPressed()
{
	Super::Attack();

	if (!(Attributes && Attributes->GetStamina() > Attributes->GetAttackCost()))
	{
		return;
	}

	// 지상 공격: CanAttack()으로 상태를 확인하고 콤보 공격을 실행
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
	// 공중 공격: 점프 공격이 가능한 상태일 때만 실행
	else if (GetCharacterMovement()->IsFalling() && CanAttack() && bCanJumpAttack)
	{
		PlayJumpAttackMontage();
		ActionState = EActionState::EAS_Attacking;
		bCanJumpAttack = false; // 점프 공격은 한 번만 가능하도록 제한

		if (Attributes && GirlOverlay)
		{
			Attributes->UseStamina(Attributes->GetAttackCost());
			GirlOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
		}
	}
}

void AGirl::Keyboard_A_Gamepad_Y_KeyPressed()
{
	if (IsOccupied())
		return;
}

void AGirl::Up_KeyPressed()
{
}

// 데미지를 받는 로직을 처리하는 함수
float AGirl::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// '피격 후 무적' 상태이거나 '회피 무적' 상태일 경우 데미지를 받지 않음
	if (bIsInvincible || bIsDodgeInvincible)
	{
		return 0.f;
	}

	HandleDamage(DamageAmount);
	SetHUDHealth();

	// 피격 후 잠시 동안의 추가 무적 시간을 부여하여, 연속적인 피격으로 인한 불쾌한 경험을 방지
	if (Attributes && Attributes->GetHealthPercent() > 0.f)
	{
		// 여기서는 '피격 후 무적'인 bIsInvincible만 true로 설정
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

// 피격 당했을 때의 리액션을 처리하는 인터페이스 함수
void AGirl::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	// '회피 무적'(bIsDodgeInvincible) 상태일 때만 피격 리액션을 무시
	// 이제 ActionState가 Dodge여도, bIsDodgeInvincible이 false이면 (즉, 0.5초가 지났으면)
	// 이 코드가 정상적으로 실행되어 피격 리액션을 보여줌
	if (bIsDodgeInvincible)
	{
		return;
	}

	Super::GetHit_Implementation(ImpactPoint, Hitter);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision); // 피격 시 공격 판정은 비활성화

	if (Attributes && Attributes->GetHealthPercent() > 0.f)
	{
		GetCharacterMovement()->Velocity = FVector::ZeroVector; // 피격 시 움직임을 멈춤
		ActionState = EActionState::EAS_HitReaction; // 상태를 '피격 중'으로 변경
	}

	ResetComboCounter(); // 피격 시 콤보를 초기화
}

// 아이템과 겹쳤을 때, 해당 아이템의 정보를 저장하는 인터페이스 함수
void AGirl::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

// 체력 회복 아이템을 획득했을 때 호출되는 인터페이스 함수
void AGirl::AddHealth(AHealth_Item* Health_Item)
{
	if (Attributes && GirlOverlay)
	{
		Attributes->AddHealth(Health_Item->GetHealth());
	}

	SetHUDHealth(); // 체력 변경 후 즉시 HUD에 반영
}

// Soul 획득 시 호출되지만 사용되지 않으므로 주석처리
//void AGirl::AddSouls(ASoul* Soul)
//{
//	if (Attributes && GirlOverlay)
//	{
//		Attributes->AddSouls(Soul->GetSouls());
//		GirlOverlay->SetSouls(Attributes->GetSouls());
//	}
//}

// Gold 획득 시 호출되지만 사용되지 않으므로 주석처리
//void AGirl::AddGold(ATreasure* Treasure)
//{
//	if (Attributes && GirlOverlay)
//	{
//		Attributes->AddGold(Treasure->GetGold());
//		GirlOverlay->SetGold(Attributes->GetGold());
//	}
//}

// 피격 애니메이션이 끝났을 때 호출되어 상태를 복원하는 함수 (주로 Anim Notify에서 호출)
void AGirl::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

// HUD와 캐릭터의 데이터를 연동하는 초기화 함수
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
				// 게임 시작 시 체력, 스태미나 등의 초기값을 HUD에 설정
				GirlOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
				GirlOverlay->SetStaminaBarPercent(1.f);
				//GirlOverlay->SetGold(0);
				//GirlOverlay->SetSouls(0);
			}
		}
	}
}

// HUD의 체력 바를 업데이트하는 함수
void AGirl::SetHUDHealth()
{
	if (GirlOverlay && Attributes)
	{
		GirlOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

// 캐릭터가 죽었을 때의 로직을 처리하는 함수
void AGirl::Die_Implementation()
{
	Super::Die_Implementation();
	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();
}

// 회피 애니메이션이 끝났을 때 호출되는 함수 (주로 Anim Notify에서 호출)
void AGirl::DodgeEnd()
{
	Super::DodgeEnd();
	ActionState = EActionState::EAS_Unoccupied;
	ResetComboCounter();

	// 애니메이션이 타이머보다 먼저 끝나는 경우를 대비하여, 여기서도 효과를 강제로 종료함
	if (GetWorld()->GetTimerManager().IsTimerActive(DodgeEffectTimerHandle))
	{
		EndDodgeEffects();
	}
}

// 현재 공격이 가능한 상태인지 확인하는 유틸리티 함수
bool AGirl::CanAttack()
{
	// 대기 상태이고, 무기를 장착하고 있을 때만 공격이 가능
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

// 공격 애니메이션이 끝났을 때 호출되는 함수 (주로 Anim Notify에서 호출)
void AGirl::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool AGirl::HasEnoughStamina()
{
	return Attributes && Attributes->GetStamina() > Attributes->GetDodgeCost();
}

// 현재 캐릭터가 다른 행동을 할 수 없는 '바쁜' 상태인지 확인하는 함수
bool AGirl::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}

bool AGirl::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

// 피격 후 무적 상태를 해제하는 함수 (TimerManager에 의해 호출)
void AGirl::EndInvincibility()
{
	bIsInvincible = false;
	StopFlashing(); // 깜빡임 효과도 함께 중지
}

// 메시 깜빡임 효과를 위한 토글 함수 (TimerManager에 의해 반복 호출)
void AGirl::ToggleMeshVisibility()
{
	if (GetMesh())
	{
		GetMesh()->SetVisibility(!GetMesh()->IsVisible()); // 현재 가시성을 반전
	}
}

// 메시 깜빡임 효과를 중지하고, 원래대로 보이도록 설정하는 함수
void AGirl::StopFlashing()
{
	GetWorld()->GetTimerManager().ClearTimer(FlashTimerHandle); // 깜빡임 타이머 해제
	if (GetMesh())
	{
		GetMesh()->SetVisibility(true); // 메쉬를 항상 보이도록 설정 (깜빡임 중단 후)
	}
}

// 회피 효과(무적/Overlap)를 해제하는 함수 (TimerManager 또는 Anim Notify에 의해 호출)
void AGirl::EndDodgeEffects()
{
	GetWorld()->GetTimerManager().ClearTimer(DodgeEffectTimerHandle);

	// 이 함수는 '회피 무적'과 충돌 설정만 관리
	// 복잡한 조건 없이 bIsDodgeInvincible을 false로 설정하여 회피 무적을 끄고,
	// 충돌 설정을 원래대로 되돌림
	bIsDodgeInvincible = false;

	if (GetCapsuleComponent())
	{
		// 충돌 설정을 원래대로 복원
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	}
}

// 일시정지 메뉴를 호출하는 함수
void AGirl::PauseGame()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		if (PauseMenuWidgetClass)
		{
			// 이미 메뉴가 열려있으면 중복 실행을 방지
			if (PauseMenuInstance && PauseMenuInstance->IsInViewport())
			{
				return;
			}

			PauseMenuInstance = CreateWidget<UUserWidget>(PlayerController, PauseMenuWidgetClass);
			if (PauseMenuInstance)
			{
				PauseMenuInstance->AddToViewport();
				PlayerController->bShowMouseCursor = true;

				// 1. 입력 모드를 UI 전용으로 변경하여, 게임 캐릭터가 움직이지 않도록 함
				FInputModeUIOnly InputModeData;
				InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PlayerController->SetInputMode(InputModeData);

				// 2. 게임 월드의 시간을 정지시킴
				UGameplayStatics::SetGamePaused(GetWorld(), true);
			}
		}
	}
}
