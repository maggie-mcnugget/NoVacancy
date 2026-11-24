#include "WinZone.h"

#include "Components/BoxComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

AWinZone::AWinZone()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create the collision box
    WinTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("WinTrigger"));

    RootComponent = WinTrigger;

    // Configure collision
    WinTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    WinTrigger->SetCollisionObjectType(ECC_WorldDynamic);
    WinTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    WinTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    WinTrigger->SetGenerateOverlapEvents(true);
}

void AWinZone::BeginPlay()
{
    Super::BeginPlay();

    // Listen for something entering the win zone
    WinTrigger->OnComponentBeginOverlap.AddDynamic(
        this,
        &AWinZone::OnWinZoneBeginOverlap
    );
}

void AWinZone::OnWinZoneBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    // Don't activate the win screen more than once
    if (bHasWon)
    {
        return;
    }

    // Only allow the player character to trigger the win zone
    if (!OtherActor || !OtherActor->IsA(ACharacter::StaticClass()))
    {
        return;
    }

    bHasWon = true;

    // Get the player's controller
    APlayerController* PlayerController =
        Cast<APlayerController>(OtherActor->GetInstigatorController());

    // Fall back to the world's first player controller if necessary
    if (!PlayerController)
    {
        PlayerController = GetWorld()->GetFirstPlayerController();
    }

    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("WinZone: Could not find PlayerController!"));
        return;
    }

    // Display the win screen
    if (WinScreenClass)
    {
        UUserWidget* WinScreen = CreateWidget<UUserWidget>(
            PlayerController,
            WinScreenClass
        );

        if (WinScreen)
        {
            WinScreen->AddToViewport();

            // Stop the player from moving or looking around
            PlayerController->SetIgnoreMoveInput(true);
            PlayerController->SetIgnoreLookInput(true);

            UE_LOG(LogTemp, Log, TEXT("Player escaped!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("WinZone: No WinScreenClass assigned!"));
    }
}


