// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Interfaces/PickupInterface.h"
#include "Blueprint/UserWidget.h"
#include "Girl.generated.h"

class AItem;
//class ASoul;
//class ATreasure;
class AHealth_Item;
class AWeapon;
class UGirlOverlay;

UCLASS()
class BERSERKER_GIRL_API AGirl : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

private:
	bool IsUnoccupied();
	void InitializeGirlOverlay();
	void SetHUDHealth();

	UPROPERTY(VisibleInstanceOnly)
		AItem* OverlappingItem;

	// 무적 변수 분리
	/** 피격 후 무적 상태 (연속 피격 방지) */
	bool bIsInvincible = false;

	/** 회피(Dodge) 중 무적 상태 (I-Frame 구현) */
	bool bIsDodgeInvincible = false;

	/** 무적 지속시간을 관리하는 타이머 핸들 (피격 후) */
	FTimerHandle InvincibilityTimerHandle;

	/** 무적 지속시간 */
	UPROPERTY(EditAnywhere, Category = "Combat")
		float InvincibilityDuration = 1.0f;

	/** 피격 후 무적 상태를 해제 */
	void EndInvincibility();

	UPROPERTY(EditAnywhere)
		TSubclassOf<class AWeapon> WeaponClass;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UGirlOverlay* GirlOverlay;

	bool bCanJumpAttack;
	bool bWasOnGround = true;

	//깜빡임 효과를 위한 타이머와 함수
	FTimerHandle FlashTimerHandle;
	UPROPERTY(EditAnywhere, Category = "Combat")
		float FlashInterval = 0.1f; // 깜빡임 주기
	void ToggleMeshVisibility(); // 메쉬 가시성을 토글
	void StopFlashing(); // 깜빡임 멈추기

	// 구르기 오버랩/무적 시간을 위한 변수 및 함수
	FTimerHandle DodgeEffectTimerHandle; // 구르기 오버랩/무적 시간을 관리할 타이머
	UPROPERTY(EditAnywhere, Category = "Combat")
		float DodgeDuration = 0.5f; // 구르기 중 Overlap 및 무적 유지 시간 (0.5초)
	void EndDodgeEffects(); // 구르기 오버랩 및 무적을 해제

public:
	AGirl();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

	virtual void SetOverlappingItem(AItem* Item) override;
	//virtual void AddSouls(ASoul* Soul) override;
	//virtual void AddGold(ATreasure* Treasure) override;
	virtual void AddHealth(AHealth_Item* Health_Item) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }

protected:
	virtual void BeginPlay() override;

	void Keyboard_X_Gamepad_A_KeyPressed();
	void Keyboard_X_Gamepad_A_KeyReleased();

	void Keyboard_S_Gamepad_B_KeyPressed();
	void Keyboard_Z_Gamepad_X_KeyPressed();
	void Keyboard_A_Gamepad_Y_KeyPressed();

	void Up_KeyPressed();

	UFUNCTION(BlueprintCallable)
		virtual void Die_Implementation() override;
	//virtual void Die() override;

	bool HasEnoughStamina();
	bool IsOccupied();

	UFUNCTION(BlueprintCallable)
		void HitReactEnd();

	virtual void DodgeEnd() override;

	UFUNCTION(BlueprintCallable)
		virtual bool CanAttack() override;

	virtual void AttackEnd() override;

	void PauseGame();
	UPROPERTY(EditAnywhere, Category = "UI")
		TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY()
		UUserWidget* PauseMenuInstance;
};
