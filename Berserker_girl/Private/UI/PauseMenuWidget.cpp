#include "UI/PauseMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"

void UPauseMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (Custom_Resume)
    {
        Custom_Resume->SetKeyboardFocus();
        Custom_Resume->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
    }

    if (Custom_Main)
    {
        Custom_Main->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnMainClicked);
    }

    if (Custom_System)
    {
        Custom_System->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnSystemClicked);
    }

    if (Custom_Quit)
    {
        Custom_Quit->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitClicked);
    }
}

void UPauseMenuWidget::OnResumeClicked()
{
    if (ClickSound)
    {
        UGameplayStatics::PlaySound2D(this, ClickSound);
    }

    APlayerController* PlayerController = GetOwningPlayer();
    if (PlayerController)
    {
        UGameplayStatics::SetGamePaused(this, false);
        FInputModeGameOnly InputModeData;
        PlayerController->SetInputMode(InputModeData);
        PlayerController->SetShowMouseCursor(false);
    }

    RemoveFromParent();
}

void UPauseMenuWidget::OnMainClicked()
{
    if (ClickSound)
    {
        UGameplayStatics::PlaySound2D(this, ClickSound);
    }

    UGameplayStatics::SetGamePaused(this, false);

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UPauseMenuWidget::OpenMainMenu, 0.3f, false);
}

void UPauseMenuWidget::OpenMainMenu()
{
    UGameplayStatics::OpenLevel(this, FName("Start"));
}

void UPauseMenuWidget::OnSystemClicked()
{
    if (ClickSound)
    {
        UGameplayStatics::PlaySound2D(this, ClickSound);
    }

    if (OptionsWidgetClass)
    {
        APlayerController* PlayerController = GetOwningPlayer();
        if (PlayerController)
        {
            RemoveFromParent();
            CreateWidget<UUserWidget>(PlayerController, OptionsWidgetClass)->AddToViewport();
        }
    }
}

void UPauseMenuWidget::OnQuitClicked()
{
    if (ClickSound)
    {
        UGameplayStatics::PlaySound2D(this, ClickSound);
    }

    UGameplayStatics::SetGamePaused(this, false);

    FTimerHandle QuitTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(QuitTimerHandle, this, &UPauseMenuWidget::QuitTheGame, 0.3f, false);
}

void UPauseMenuWidget::QuitTheGame()
{
    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
