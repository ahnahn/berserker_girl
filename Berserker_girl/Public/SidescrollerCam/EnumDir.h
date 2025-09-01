// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class Enum_RightDir : uint8
{
    Y_Plus   UMETA(DisplayName = "Y+"),
    Y_Minus  UMETA(DisplayName = "Y-"),
    X_Plus   UMETA(DisplayName = "X+"),
    X_Minus  UMETA(DisplayName = "X-")
};

class BERSERKER_GIRL_API EnumDir
{
public:
	EnumDir();
	~EnumDir();
};
