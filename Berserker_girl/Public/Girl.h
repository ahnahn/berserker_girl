// Fill out your copyright notice in the Description page of Project Settings.



#pragma once



#include "CoreMinimal.h"

#include "BaseCharacter.h"

#include "Characters/CharacterTypes.h"

#include "Interfaces/PickupInterface.h"

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

	//������ ���� �� ���� ��������/

	bool bIsInvincible = false;

	/** ���� ���ӽð��� �����ϴ� Ÿ�̸� �ڵ� */

	FTimerHandle InvincibilityTimerHandle;

	/** ���� ���ӽð� */

	UPROPERTY(EditAnywhere, Category = "Combat")

		float InvincibilityDuration = 1.0f;



	/** ���� ���¸� ���� */

	void EndInvincibility();



	UPROPERTY(EditAnywhere)

		TSubclassOf<class AWeapon> WeaponClass;



	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))

		EActionState ActionState = EActionState::EAS_Unoccupied;



	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))

		UGirlOverlay* GirlOverlay;



	bool bCanJumpAttack;
	bool bWasOnGround = true;

	//������ ȿ���� ���� Ÿ�̸ӿ� �Լ�
	FTimerHandle FlashTimerHandle;
	UPROPERTY(EditAnywhere, Category = "Combat") float FlashInterval = 0.1f; // ������ �ֱ�
	void ToggleMeshVisibility(); // �޽� ���ü��� ���
	void StopFlashing(); // ������ ���߱�


	// ������ ������/���� �ð��� ���� ���� �� �Լ�
	FTimerHandle DodgeEffectTimerHandle; // ������ ������/���� �ð��� ������ Ÿ�̸�
	UPROPERTY(EditAnywhere, Category = "Combat") float DodgeDuration = 0.5f; // ������ �� Overlap �� ���� ���� �ð� (0.5��)
	void EndDodgeEffects(); // ������ ������ �� ������ ����

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



};