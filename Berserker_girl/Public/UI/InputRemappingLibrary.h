// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/PlayerInput.h"
#include "InputRemappingLibrary.generated.h"

UCLASS()
class BERSERKER_GIRL_API UInputRemappingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 지정된 액션 매핑의 키를 변경 (점프, 공격, 구르기)
	 * 기존에 있던 게임 패드를 제외한 모든 바인딩을 제거하고 새로운 키로 교체합니다.
	 * @param ActionName - 변경할 액션의 이름 (예: "Pad_A")
	 * @param NewKeyChord - Input Key Selector 위젯에서 선택된 새로운 키 정보 (Shift, Ctrl 등 포함)
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Remapping Functions")
		static void RebindActionKey(FName ActionName, FInputChord NewKeyChord);

	/**
	 * 지정된 축 매핑의 키를 변경 (이동)
	 * 기존에 있던 게임 패드를 제외한 특정 Scale 값을 가진 기존 바인딩만 찾아 새로운 키로 교체
	 * @param AxisName - 변경할 축의 이름 (예: "MoveRight")
	 * @param Scale - 변경할 키에 적용될 스케일 값 (예: 왼쪽 이동 -1.0, 오른쪽 이동 1.0)
	 * @param NewKey - Input Key Selector 위젯에서 선택된 새로운 키
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Remapping Functions")
		static void RebindAxisKey(FName AxisName, float Scale, FKey NewKey);

	/**
	* 지정된 액션 매핑에 현재 바인딩된 키보드 키 정보 가져오기
	* @param ActionName - 키를 찾을 액션의 이름 (예: "Pad_A")
	* @return 현재 설정된 키와 Shift, Ctrl 등 수식어 키 정보를 포함한 FInputChord
	*/
	UFUNCTION(BlueprintPure, Category = "Input Remapping Functions")
		static FInputChord GetActionKeyChord(FName ActionName);

	/**
	 * 지정된 축 매핑에 현재 바인딩된 키보드 키 정보 가져오기
	 * @param AxisName - 키를 찾을 축의 이름 (예: "MoveRight")
	 * @param Scale - 찾을 키의 스케일 값 (예: -1.0)
	 * @return 현재 설정된 키 정보
	 */
	UFUNCTION(BlueprintPure, Category = "Input Remapping Functions")
		static FKey GetAxisKey(FName AxisName, float Scale);
};
