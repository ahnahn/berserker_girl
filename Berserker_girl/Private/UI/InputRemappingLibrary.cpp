// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/InputRemappingLibrary.h"
#include "GameFramework/InputSettings.h"

void UInputRemappingLibrary::RebindActionKey(FName ActionName, FInputChord NewKeyChord)
{
	UInputSettings* Settings = GetMutableDefault<UInputSettings>();
	if (!Settings) return;

	// 기존 바인딩 제거
	TArray<FInputActionKeyMapping> OldMappings;
	Settings->GetActionMappingByName(ActionName, OldMappings);

	// 루프를 돌며 키보드/마우스 매핑만 찾아서 제거
	for (const FInputActionKeyMapping& Mapping : OldMappings)
	{
		if (!Mapping.Key.IsGamepadKey()) // 이 키가 게임패드 키가 아닐 경우에만
		{
			Settings->RemoveActionMapping(Mapping);
		}
	}

	// 새 바인딩 추가
	FInputActionKeyMapping NewMapping(ActionName, NewKeyChord.Key, NewKeyChord.bShift, NewKeyChord.bCtrl, NewKeyChord.bAlt, NewKeyChord.bCmd);
	Settings->AddActionMapping(NewMapping, true);
	Settings->SaveKeyMappings();
}

void UInputRemappingLibrary::RebindAxisKey(FName AxisName, float Scale, FKey NewKey)
{
	UInputSettings* Settings = GetMutableDefault<UInputSettings>();
	if (!Settings) return;

	// 기존 바인딩 제거
	TArray<FInputAxisKeyMapping> OldMappings;
	Settings->GetAxisMappingByName(AxisName, OldMappings);

	// 루프를 돌며 키보드/마우스이면서 Scale 값이 일치하는 매핑만 찾아서 제거
	for (const FInputAxisKeyMapping& Mapping : OldMappings)
	{
		// 키가 게임패드 키가 아니고 Scale 값도 일치하는 경우
		if (!Mapping.Key.IsGamepadKey() && FMath::IsNearlyEqual(Mapping.Scale, Scale))
		{
			Settings->RemoveAxisMapping(Mapping);
			break; // 키보드에서는 보통 스케일당 하나의 키만 존재하므로 찾으면 루프 종료
		}
	}

	// 새 바인딩 추가
	FInputAxisKeyMapping NewMapping(AxisName, NewKey, Scale);
	Settings->AddAxisMapping(NewMapping, true);
	Settings->SaveKeyMappings();
}

FInputChord UInputRemappingLibrary::GetActionKeyChord(FName ActionName)
{
	const UInputSettings* Settings = GetDefault<UInputSettings>();
	if (Settings)
	{
		TArray<FInputActionKeyMapping> Mappings;
		Settings->GetActionMappingByName(ActionName, Mappings);

		// 키보드 매핑을 찾기
		for (const FInputActionKeyMapping& Mapping : Mappings)
		{
			if (!Mapping.Key.IsGamepadKey())
			{
				// 찾으면 FInputChord 구조체를 만들어서 반환
				return FInputChord(Mapping.Key, Mapping.bShift, Mapping.bCtrl, Mapping.bAlt, Mapping.bCmd);
			}
		}
	}
	// 못 찾았으면 빈 FInputChord를 반환
	return FInputChord();
}

FKey UInputRemappingLibrary::GetAxisKey(FName AxisName, float Scale)
{
	const UInputSettings* Settings = GetDefault<UInputSettings>();
	if (Settings)
	{
		TArray<FInputAxisKeyMapping> Mappings;
		Settings->GetAxisMappingByName(AxisName, Mappings);

		// 키보드 매핑과 Scale이 일치하는 것을 찾기
		for (const FInputAxisKeyMapping& Mapping : Mappings)
		{
			if (!Mapping.Key.IsGamepadKey() && FMath::IsNearlyEqual(Mapping.Scale, Scale))
			{
				return Mapping.Key;
			}
		}
	}
	// 못 찾았면 EKeys::Invalid를 반환
	return EKeys::Invalid;
}
