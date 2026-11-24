
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "PatrolEnemy.generated.h"

class AActor;

UCLASS()
class NOVACANCY_API APatrolEnemy : public ACharacter
{
    GENERATED_BODY()

public:
    APatrolEnemy();

protected:
    virtual void BeginPlay() override;

    // Actors placed in the level that define the patrol route
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Patrol")
    TArray<TObjectPtr<AActor>> PatrolPoints;

    // Which patrol point the enemy is currently moving toward
    int32 CurrentPatrolIndex = 0;

    // How close the enemy needs to get before moving to the next point
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Patrol")
    float PatrolAcceptanceRadius = 100.0f;

    // How often we check whether the enemy has reached its point
    FTimerHandle PatrolTimerHandle;

    // Start moving toward the current patrol point
    void MoveToPatrolPoint();

    //Checks wether we have reached the current patrol point
	void CheckPatrolProgress();

    // Used to move to the next point in the patrol route
    void MoveToNextPatrolPoint();
};

