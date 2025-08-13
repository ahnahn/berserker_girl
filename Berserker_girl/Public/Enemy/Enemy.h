// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class UPawnSensingComponent;

UCLASS()
class BERSERKER_GIRL_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	/** <AActor> */
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	/** </AActor> */

	/** <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/** </IHitInterface> */

protected:
	/** <AActor> */
	virtual void BeginPlay() override;
	/** </AActor> */

	/** <ABaseCharacter> */
	//virtual void Die() override;
	virtual void Die_Implementation() override;

	void SpawnSoul();
	void SpawnHealthItem();
	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void AttackEnd() override;
	virtual void HandleDamage(float DamageAmount) override;
	/** </ABaseCharacter> */

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
		float HitReactionDelay = 0.2f;


	// 적이 주인공을 한 번이라도 봤는지 여부
	bool bHasSeenPlayer = false;

	// 주인공 마지막으로 본 방향
	FVector LastKnownPlayerLocation = FVector::ZeroVector;

private:

	/** AI Behavior */
	void InitializeEnemy();
	void CheckPatrolTarget();
	void CheckCombatTarget();
	void PatrolTimerFinished();
	void HideHealthBar();
	void ShowHealthBar();
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();
	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsInsideAttackRadius();
	bool IsChasing();
	bool IsAttacking();
	bool IsDead();
	bool IsEngaged();
	void ClearPatrolTimer();
	void StartAttackTimer();
	void ClearAttackTimer();
	bool InTargetRange(AActor* Target, double Radius);
	void MoveToTarget(AActor* Target);
	AActor* ChoosePatrolTarget();
	void SpawnDefaultWeapon();


	UFUNCTION()
		void PawnSeen(APawn* SeenPawn); // Callback for OnPawnSeen in UPawnSensingComponent

	UPROPERTY(VisibleAnywhere)
		UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
		UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere, Category = Combat)
		TSubclassOf<class AWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, Category = Combat)
		TSubclassOf<class AWeapon> WeaponClass2;

	UPROPERTY(EditAnywhere, Category = Combat)
		double CombatRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = Combat)
		double AttackRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = Combat)
		double AcceptanceRadius = 50.f;

	UPROPERTY()
		class AAIController* EnemyController;

	// Current patrol target
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
		AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
		TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
		double PatrolRadius = 200.f;

	FTimerHandle PatrolTimer;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
		float PatrolWaitMin = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
		float PatrolWaitMax = 10.f;

	UPROPERTY(EditAnywhere, Category = Combat)
		float PatrollingSpeed = 125.f;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = Combat)
		float AttackMin = 0.f;

	UPROPERTY(EditAnywhere, Category = Combat)
		float AttackMax = 0.4f;


	UPROPERTY(EditAnywhere, Category = Combat)
		float ChasingSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = Combat)
		float DeathLifeSpan = 8.f;

	UPROPERTY(EditAnywhere, Category = Combat)
		TSubclassOf<class ASoul> SoulClass;

	UPROPERTY(EditAnywhere, Category = Combat)
		TSubclassOf<class AHealth_Item> AHealth_ItemClass;

	UPROPERTY()
		APawn* PlayerPawn = nullptr;
};
