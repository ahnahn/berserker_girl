// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;
class UAnimMontage;

UCLASS()
class BERSERKER_GIRL_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;






	// Combo counter
	int32 ComboCount;

	// Timer to reset combo count
	FTimerHandle ComboResetTimer;





	/** Combat */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual void Attack();
	//virtual void Die();
	UFUNCTION(BlueprintNativeEvent)
		void Die();

	void DirectionalHitReact(const FVector& ImpactPoint);
	virtual void HandleDamage(float DamageAmount);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	void DisableCapsule();
	virtual bool CanAttack();
	bool IsAlive();
	void DisableMeshCollision();

	/** Montage */
	virtual int32 PlayAttackComboMontage();
	virtual void ResetComboCounter();


	void PlayHitReactMontage(const FName& SectionName);
	virtual int32 PlayAttackMontage();
	virtual int32 PlayDeathMontage();
	virtual void PlayDodgeMontage();
	virtual void PlayJumpAttackMontage();
	void StopAttackMontage();

	UFUNCTION(BlueprintCallable)
		FVector GetTranslationWarpTarget();

	UFUNCTION(BlueprintCallable)
		FVector GetRotationWarpTarget();

	UFUNCTION(BlueprintCallable)
		virtual void AttackEnd();

	UFUNCTION(BlueprintCallable)
		virtual void DodgeEnd();

	UFUNCTION(BlueprintCallable)
		void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	UPROPERTY(VisibleAnywhere, Category = Weapon)
		AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, Category = Weapon)
		AWeapon* EquippedWeapon2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAttributeComponent* Attributes;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
		AActor* CombatTarget;

	UPROPERTY(EditAnywhere, Category = Combat)
		double WarpTargetDistance = 75.f;

	UPROPERTY(BlueprintReadOnly)
		TEnumAsByte<EDeathPose> DeathPose;



private:

	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);

	UPROPERTY(EditAnywhere, Category = Combat)
		USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = Combat)
		UParticleSystem* HitParticles;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
		UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
		UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
		UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
		UAnimMontage* DodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
		UAnimMontage* JumpAttackMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
		TArray<FName> AttackMontageSections;

	UPROPERTY(EditAnywhere, Category = Combat)
		TArray<FName> DeathMontageSections;

public:
	FORCEINLINE TEnumAsByte<EDeathPose> GetDeathPose() const { return DeathPose; }
};