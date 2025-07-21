// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	ComboCount = 0;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{


	if (IsAlive() && Hitter)
	{
		//DirectionalHitReact(Hitter->GetActorLocation());
		DirectionalHitReact(Hitter->GetActorForwardVector());
	}
	else Die_Implementation();

	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}

void ABaseCharacter::Attack()
{
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		CombatTarget = nullptr;
	}
}

void ABaseCharacter::Die_Implementation()
{
	Tags.Add(FName("Dead"));
	PlayDeathMontage();
}

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

//void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
//{
//	const FVector Forward = GetActorForwardVector();
//	// Lower Impact Point to the Enemy's Actor Location Z
//	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
//	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();
//
//	// Forward * ToHit = |Forward||ToHit| * cos(theta)
//	// |Forward| = 1, |ToHit| = 1, so Forward * ToHit = cos(theta)
//	const double CosTheta = FVector::DotProduct(Forward, ToHit);
//	// Take the inverse cosine (arc-cosine) of cos(theta) to get theta
//	double Theta = FMath::Acos(CosTheta);
//	// convert from radians to degrees
//	Theta = FMath::RadiansToDegrees(Theta);
//
//	// if CrossProduct points down, Theta should be negative
//	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
//	if (CrossProduct.Z < 0)
//	{
//		Theta *= -1.f;
//	}
//
//	FName Section("FromBack");
//
//	if (Theta < 0.f)
//	{
//		Section = FName("FromFront");
//	}
//
//	PlayHitReactMontage(Section);
//
//}
void ABaseCharacter::DirectionalHitReact(const FVector& HitterForward)
{
	const FVector Forward = GetActorForwardVector();

	// Calculate the dot product of the character's forward vector and the hitter's forward vector
	const double DotProduct = FVector::DotProduct(Forward, HitterForward);

	FName Section("FromFront");

	// If the dot product is positive, both characters are facing the same direction
	// If the dot product is negative, the characters are facing opposite directions
	if (DotProduct > 0.f)
	{
		Section = FName("FromBack");
	}

	PlayHitReactMontage(Section);
}
void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			HitSound,
			ImpactPoint
		);
	}
}

void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles && GetWorld())
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			HitParticles,
			ImpactPoint
		);
	}
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
	}
}

void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) return -1;
	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}


int32 ABaseCharacter::PlayAttackMontage()
{
	return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}

int32 ABaseCharacter::PlayAttackComboMontage()
{
	if (ComboCount >= AttackMontageSections.Num())
	{
		ComboCount = 0;  // Reset if we've gone through all the combos.
	}

	FName SectionName = AttackMontageSections[ComboCount];

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(AttackMontage);
		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
		++ComboCount;  // Increase combo count after successful attack.

		// Reset the timer for resetting the combo.
		GetWorld()->GetTimerManager().ClearTimer(ComboResetTimer);
		GetWorld()->GetTimerManager().SetTimer(ComboResetTimer, this, &ABaseCharacter::ResetComboCounter, 0.9f);

		return ComboCount;
	}
	return -1;
}

void ABaseCharacter::ResetComboCounter()
{
	ComboCount = 0;
}

int32 ABaseCharacter::PlayDeathMontage()
{
	const int32 Selection = PlayRandomMontageSection(DeathMontage, DeathMontageSections);
	TEnumAsByte<EDeathPose> Pose(Selection);
	if (Pose < EDeathPose::EDP_MAX)
	{
		DeathPose = Pose;
	}

	return Selection;
}

void ABaseCharacter::PlayDodgeMontage()
{
	PlayMontageSection(DodgeMontage, FName("Default"));
}

void ABaseCharacter::PlayJumpAttackMontage()
{
	PlayMontageSection(JumpAttackMontage, FName("Default"));
}

void ABaseCharacter::StopAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.25f, AttackMontage);
	}
}

FVector ABaseCharacter::GetTranslationWarpTarget()
{
	if (CombatTarget == nullptr) return FVector();

	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector Location = GetActorLocation();

	FVector TargetToMe = (Location - CombatTargetLocation).GetSafeNormal();
	TargetToMe *= WarpTargetDistance;

	return CombatTargetLocation + TargetToMe;

}

FVector ABaseCharacter::GetRotationWarpTarget()
{
	if (CombatTarget)
	{
		return CombatTarget->GetActorLocation();
	}
	return FVector();
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool ABaseCharacter::CanAttack()
{
	return false;
}

bool ABaseCharacter::IsAlive()
{
	return Attributes && Attributes->IsAlive();
}

void ABaseCharacter::DisableMeshCollision()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::DodgeEnd()
{
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		///EquippedWeapon->IgnoreActors.Empty();
		if (CollisionEnabled == ECollisionEnabled::QueryOnly)
		{
			EquippedWeapon->DamagedActors.Empty();
		}
	}
	if (EquippedWeapon2 && EquippedWeapon2->GetWeaponBox())
	{
		EquippedWeapon2->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		//EquippedWeapon2->IgnoreActors.Empty();
		if (CollisionEnabled == ECollisionEnabled::QueryOnly)
		{
			EquippedWeapon2->DamagedActors.Empty();
		}
	}

}