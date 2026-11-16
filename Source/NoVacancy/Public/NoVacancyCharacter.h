
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "InputActionValue.h"
#include "NoVacancyCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UPrimitiveComponent;
class UUserWidget;

UCLASS()
class NOVACANCY_API ANoVacancyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ANoVacancyCharacter();

    // Returns how many keys the player has collected
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetKeysCollected() const;

protected:
    virtual void BeginPlay() override;

    // First-person camera
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FirstPersonCamera;

    // Input assets assigned through the child Blueprint
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* CrouchAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* InteractAction;

    // The interactable object currently within reach
    UPROPERTY()
    TObjectPtr<UPrimitiveComponent> CurrentInteractableComponent;

    // Number of keys the player has collected
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 KeysCollected = 0;

    // Interaction prompt widget
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> InteractPromptClass;

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> InteractPromptWidget;

    // Key requirement message widget
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> KeyMessageClass;

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> KeyMessageWidget;

    // Timer controlling how long the key message stays visible
    FTimerHandle KeyMessageTimer;

    // Controls the fading animation
    float KeyMessageFadeTime = 1.0f;
    float KeyMessageFadeElapsed = 0.0f;
    bool bFadingKeyMessage = false;

    // Movement functions
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void ToggleCrouch();

    // Interaction functions
    void Interact();

    // Key message functions
    void ShowKeyMessage();
    void FadeOutKeyMessage();

    UFUNCTION()
    void OnInteractionBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionEndOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

public:
    virtual void SetupPlayerInputComponent(
        UInputComponent* PlayerInputComponent) override;
};

