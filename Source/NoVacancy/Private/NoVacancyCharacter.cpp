
#include "NoVacancyCharacter.h"

#include "TimerManager.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"


int32 ANoVacancyCharacter::GetKeysCollected() const
{
    return KeysCollected;
}

ANoVacancyCharacter::ANoVacancyCharacter()
{
    // Create the first-person camera
    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(
        TEXT("FirstPersonCamera"));

    // Attach the camera to the character's capsule
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());

    // Position the camera around the character's eye level
    FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));

    // The camera follows the character's rotation
    FirstPersonCamera->bUsePawnControlRotation = true;

    // Allow crouching, but don't allow jumping
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
    JumpMaxCount = 0;

    // Allow the character capsule to detect overlapping hitboxes
    GetCapsuleComponent()->SetGenerateOverlapEvents(true);
}

void ANoVacancyCharacter::BeginPlay()
{
    Super::BeginPlay();
    // Create the interaction prompt
    if (InteractPromptClass)
    {
        InteractPromptWidget = CreateWidget<UUserWidget>(
            GetWorld(),
            InteractPromptClass
        );

        if (InteractPromptWidget)
        {
            // Add the prompt to the screen
            InteractPromptWidget->AddToViewport();

            // Hide it until the player finds an interactable object
            InteractPromptWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    // Create the key requirement message
    if (KeyMessageClass)
    {
        KeyMessageWidget = CreateWidget<UUserWidget>(
            GetWorld(),
            KeyMessageClass
        );

        if (KeyMessageWidget)
        {
            // Add the message to the screen
            KeyMessageWidget->AddToViewport();

            // Hide it until needed
            KeyMessageWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    // Listen for interactable objects entering and leaving the character capsule
    GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(
        this, &ANoVacancyCharacter::OnInteractionBeginOverlap);

    GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(
        this, &ANoVacancyCharacter::OnInteractionEndOverlap);

    // Add the input mapping context to the player
    if (APlayerController* PlayerController =
        Cast<APlayerController>(GetController()))
    {
        if (ULocalPlayer* LocalPlayer =
            PlayerController->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                if (DefaultMappingContext)
                {
                    Subsystem->AddMappingContext(DefaultMappingContext, 0);
                }
            }
        }
    }
}

void ANoVacancyCharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Get the Enhanced Input component
    if (UEnhancedInputComponent* EnhancedInput =
        Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Bind movement
        if (MoveAction)
        {
            EnhancedInput->BindAction(
                MoveAction,
                ETriggerEvent::Triggered,
                this,
                &ANoVacancyCharacter::Move);
        }

        // Bind camera look
        if (LookAction)
        {
            EnhancedInput->BindAction(
                LookAction,
                ETriggerEvent::Triggered,
                this,
                &ANoVacancyCharacter::Look);
        }

        // Bind crouch
        if (CrouchAction)
        {
            EnhancedInput->BindAction(
                CrouchAction,
                ETriggerEvent::Started,
                this,
                &ANoVacancyCharacter::ToggleCrouch);
        }

        // Bind interaction
        if (InteractAction)
        {
            EnhancedInput->BindAction(
                InteractAction,
                ETriggerEvent::Started,
                this,
                &ANoVacancyCharacter::Interact);
        }
    }
}

void ANoVacancyCharacter::Move(const FInputActionValue& Value)
{
    // Get the 2D movement input
    FVector2D MovementVector = Value.Get<FVector2D>();

    // Get the direction the controller is facing
    if (Controller)
    {
        const FRotator ControlRotation = Controller->GetControlRotation();

        // Only use the horizontal direction
        const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

        const FVector ForwardDirection =
            FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        const FVector RightDirection =
            FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // Move forward/backward and left/right
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ANoVacancyCharacter::Look(const FInputActionValue& Value)
{
    // Get the 2D mouse input
    FVector2D LookVector = Value.Get<FVector2D>();

    // Apply mouse movement to the controller
    AddControllerYawInput(LookVector.X);
    AddControllerPitchInput(LookVector.Y);
}

void ANoVacancyCharacter::ToggleCrouch()
{
    // If already crouching, stand up
    if (bIsCrouched)
    {
        UnCrouch();
    }
    // Otherwise, crouch
    else
    {
        Crouch();
    }
}

void ANoVacancyCharacter::OnInteractionBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // Check whether the overlapping component is tagged as interactable
    if (OtherComp && OtherComp->ComponentHasTag(FName("Interactable")))
    {
        CurrentInteractableComponent = OtherComp;

        // Show the interaction prompt
        if (InteractPromptWidget)
        {
            InteractPromptWidget->SetVisibility(ESlateVisibility::Visible);
        }

        UE_LOG(LogTemp, Log, TEXT("Interactable object detected: %s"),
            *OtherActor->GetName());
    }
}

void ANoVacancyCharacter::OnInteractionEndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    // Check whether the player is leaving the currently selected interactable
    if (CurrentInteractableComponent == OtherComp)
    {
        CurrentInteractableComponent = nullptr;

        // Hide the interaction prompt
        if (InteractPromptWidget)
        {
            InteractPromptWidget->SetVisibility(ESlateVisibility::Hidden);
        }

        UE_LOG(LogTemp, Log, TEXT("Left interactable object"));
    }
}


void ANoVacancyCharacter::Interact()
{
    // Check whether we are currently near an interactable object
    if (CurrentInteractableComponent)
    {
        // Get the actor that owns the hitbox
        AActor* InteractableActor = CurrentInteractableComponent->GetOwner();

        if (InteractableActor)
        {
            // Check whether this object is a key
            if (InteractableActor->ActorHasTag(FName("Key")))
            {
                // Increase the player's collected key count
                KeysCollected++;

                UE_LOG(LogTemp, Log, TEXT("Key collected! Total keys: %d"),
                    KeysCollected);

                // Show a temporary confirmation on screen
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(
                        -1,
                        2.0f,
                        FColor::Green,
                        FString::Printf(TEXT("Key Collected! Keys: %d"),
                            KeysCollected));
                }

                // Clear the current interactable reference
                CurrentInteractableComponent = nullptr;

                // Hide the interaction prompt
                if (InteractPromptWidget)
                {
                    InteractPromptWidget->SetVisibility(
                        ESlateVisibility::Hidden);
                }

                // Remove the key from the level
                InteractableActor->Destroy();
            }

            // Check whether this object is a regular interior door
            else if (InteractableActor->ActorHasTag(FName("InteriorDoor")))
            {
                // Check whether the door is currently open
                if (InteractableActor->ActorHasTag(FName("Opened")))
                {
                    // Find the CloseDoor event
                    UFunction* CloseDoorFunction = InteractableActor->FindFunction(
                        FName("CloseDoor")
                    );

                    if (CloseDoorFunction)
                    {
                        // Play the closing animation
                        InteractableActor->ProcessEvent(
                            CloseDoorFunction,
                            nullptr
                        );

                        // Remove the opened tag
                        InteractableActor->Tags.Remove(FName("Opened"));
                    }
                }
                else
                {
                    // Find the OpenDoor event
                    UFunction* OpenDoorFunction = InteractableActor->FindFunction(
                        FName("OpenDoor")
                    );

                    if (OpenDoorFunction)
                    {
                        // Play the opening animation
                        InteractableActor->ProcessEvent(
                            OpenDoorFunction,
                            nullptr
                        );

                        // Add the opened tag
                        InteractableActor->Tags.AddUnique(FName("Opened"));
                    }
                }
            }

            // Check whether this object is the exit door
            else if (InteractableActor->ActorHasTag(FName("ExitDoor")))
            {
                // Check whether the player has collected all 3 keys
                if (KeysCollected >= 3)
                {
                    // Check whether the door has already been opened
                    if (InteractableActor->ActorHasTag(FName("Opened")))
                    {
                        return;
                    }

                    // Find the OpenDoor event inside the door Blueprint
                    UFunction* OpenDoorFunction = InteractableActor->FindFunction(
                        FName("OpenDoor")
                    );

                    if (OpenDoorFunction)
                    {
                        // Play the door opening animation
                        InteractableActor->ProcessEvent(
                            OpenDoorFunction,
                            nullptr
                        );

                        // Mark the door as opened so it cannot be opened again
                        InteractableActor->Tags.AddUnique(FName("Opened"));

                        // Clear the current interactable reference
                        CurrentInteractableComponent = nullptr;

                        // Hide the interaction prompt
                        if (InteractPromptWidget)
                        {
                            InteractPromptWidget->SetVisibility(
                                ESlateVisibility::Hidden);
                        }

                        UE_LOG(LogTemp, Log, TEXT("Exit unlocked! Door opening."));
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error,
                            TEXT("Could not find OpenDoor event in exit door Blueprint!"));
                    }
                }
                else
                {
                    if (KeyMessageWidget)
                    {
                        // Find the text widget inside WBP_KeyMessage
                        UTextBlock* MessageText = Cast<UTextBlock>(
                            KeyMessageWidget->GetWidgetFromName(
                                TEXT("KeyMessageText")
                            )
                        );

                        // Update the message with the current key count
                        if (MessageText)
                        {
                            MessageText->SetText(
                                FText::FromString(
                                    FString::Printf(
                                        TEXT("You need more keys! %d/3 Keys"),
                                        KeysCollected
                                    )
                                )
                            );
                        }

                        // Show the message and start its timer
                        ShowKeyMessage();
                    }
                }
            }

        }
    }
}

void ANoVacancyCharacter::ShowKeyMessage()
{
    if (!KeyMessageWidget)
    {
        return;
    }

    // Stop any currently playing fade animation
    UFunction* StopFadeFunction = KeyMessageWidget->FindFunction(
        FName("StopFadeOut")
    );

    if (StopFadeFunction)
    {
        KeyMessageWidget->ProcessEvent(StopFadeFunction, nullptr);
    }

    // Make the message visible again
    KeyMessageWidget->SetVisibility(ESlateVisibility::Visible);
    KeyMessageWidget->SetRenderOpacity(1.0f);

    // Cancel any previous timer
    GetWorldTimerManager().ClearTimer(KeyMessageTimer);

    // Start a fresh 3-second timer
    GetWorldTimerManager().SetTimer(
        KeyMessageTimer,
        this,
        &ANoVacancyCharacter::FadeOutKeyMessage,
        3.0f,
        false
    );
}

void ANoVacancyCharacter::FadeOutKeyMessage()
{
    if (!KeyMessageWidget)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Playing key message fade"));

    // Find the PlayFadeOut function inside WBP_KeyMessage
    UFunction* FadeFunction = KeyMessageWidget->FindFunction(
        FName("PlayFadeOut")
    );

    if (FadeFunction)
    {
        // Run the Widget Blueprint function
        KeyMessageWidget->ProcessEvent(FadeFunction, nullptr);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Could not find PlayFadeOut function"));
    }
}



