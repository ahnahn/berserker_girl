#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;
class USoundBase;

UCLASS()
class BERSERKER_GIRL_API UPauseMenuWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
        UButton* Custom_Resume;

    UPROPERTY(meta = (BindWidget))
        UButton* Custom_Main;

    UPROPERTY(meta = (BindWidget))
        UButton* Custom_System;

    UPROPERTY(meta = (BindWidget))
        UButton* Custom_Quit;

    UFUNCTION()
        void OnResumeClicked();

    UFUNCTION()
        void OnMainClicked();

    UFUNCTION()
        void OnSystemClicked();

    UFUNCTION()
        void OnQuitClicked();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI SFX")
        USoundBase* ClickSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Widgets")
        TSubclassOf<UUserWidget> OptionsWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI Widgets")
        TSubclassOf<UUserWidget> QuitConfirmWidgetClass;
private:
    void OpenMainMenu();
    void QuitTheGame();
};
