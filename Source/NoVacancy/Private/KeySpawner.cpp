
#include "KeySpawner.h"

#include "Engine/World.h"

AKeySpawner::AKeySpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AKeySpawner::BeginPlay()
{
    Super::BeginPlay();

    // Make sure a key Blueprint has been assigned
    if (!KeyClass)
    {
        UE_LOG(LogTemp, Error, TEXT("KeySpawner: No KeyClass assigned!"));
        return;
    }

    // Make a copy of the available spawn locations
    TArray<TObjectPtr<AActor>> AvailableLocations = SpawnLocations;

    // Check that we have enough locations
    if (AvailableLocations.Num() < NumberOfKeysToSpawn)
    {
        UE_LOG(LogTemp, Error,
            TEXT("KeySpawner: Not enough spawn locations!"));
        return;
    }

    // Spawn the requested number of keys
    for (int32 i = 0; i < NumberOfKeysToSpawn; i++)
    {
        // Randomly select one of the remaining locations
        int32 RandomIndex = FMath::RandRange(
            0,
            AvailableLocations.Num() - 1
        );

        AActor* SelectedLocation = AvailableLocations[RandomIndex];

        // Remove this location so it cannot be selected again
        AvailableLocations.RemoveAt(RandomIndex);

        if (SelectedLocation)
        {
            // Set up the spawn parameters
            FActorSpawnParameters SpawnParameters;

            SpawnParameters.SpawnCollisionHandlingOverride =
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            // Spawn the key at the selected location
            GetWorld()->SpawnActor<AActor>(
                KeyClass,
                SelectedLocation->GetActorTransform(),
                SpawnParameters
            );

            UE_LOG(LogTemp, Log,
                TEXT("Key spawned at location: %s"),
                *SelectedLocation->GetName());
        }
    }
}


