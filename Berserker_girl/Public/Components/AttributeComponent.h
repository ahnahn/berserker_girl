// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BERSERKER_GIRL_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void RegenStamina(float DeltaTime);

	void ReceiveDamage(float Damage);
	void UseStamina(float StaminaCost);

	UFUNCTION(BlueprintCallable)
	float GetHealthPercent();

	float GetStaminaPercent();

	bool IsAlive();

	void AddSouls(int32 NumberOfSouls);
	void AddGold(int32 AmountOfGold);
	void AddHealth(float AmountOfHealth);
	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE int32 GetSouls() const { return Souls; }
	FORCEINLINE float GetDodgeCost() const { return DodgeCost; }
	FORCEINLINE float GetAttackCost() const { return AttackCost; }
	FORCEINLINE float GetJumpCost() const { return JumpCost; }
	FORCEINLINE float GetStamina() const { return Stamina; }
	FORCEINLINE float GetHealth() const { return Health; }

protected:
	virtual void BeginPlay() override;

private:
	// Current Health
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
		float Health;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
		float MaxHealth;

	// Current Stamina
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
		float Stamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
		float MaxStamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
		int32 Gold;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
		int32 Souls;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
		float DodgeCost = 2.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
		float AttackCost = 1.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
		float JumpCost = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
		float StaminaRegenRate = 8.f;
};
