
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KeySpawner.generated.h"

UCLASS()
class NOVACANCY_API AKeySpawner : public AActor
{
    GENERATED_BODY()

public:
    AKeySpawner();

protected:
    virtual void BeginPlay() override;

    // The Blueprint class of the key we want to spawn
    UPROPERTY(EditDefaultsOnly, Category = "Key Spawning")
    TSubclassOf<AActor> KeyClass;

    // The possible locations where keys can spawn
    UPROPERTY(EditInstanceOnly, Category = "Key Spawning")
    TArray<TObjectPtr<AActor>> SpawnLocations;

    // How many keys should spawn
    UPROPERTY(EditDefaultsOnly, Category = "Key Spawning")
    int32 NumberOfKeysToSpawn = 3;
};

