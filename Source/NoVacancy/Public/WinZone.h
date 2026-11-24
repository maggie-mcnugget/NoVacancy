
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WinZone.generated.h"

class UBoxComponent;
class UUserWidget;

UCLASS()
class NOVACANCY_API AWinZone : public AActor
{
    GENERATED_BODY()

public:
    AWinZone();

protected:
    virtual void BeginPlay() override;

    // The box that detects when the player enters the win zone
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Win Zone")
    UBoxComponent* WinTrigger;

    // The widget displayed when the player escapes
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Win Zone")
    TSubclassOf<UUserWidget> WinScreenClass;

    // Keeps the win screen from activating more than once
    bool bHasWon = false;

    UFUNCTION()
    void OnWinZoneBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );
};

