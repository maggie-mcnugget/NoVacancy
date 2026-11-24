
#include "PatrolEnemy.h"

#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

APatrolEnemy::APatrolEnemy()
{
    PrimaryActorTick.bCanEverTick = false;

    // Allow this character to be controlled by AI when placed in the level
    AIControllerClass = AAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // Set a slower walking speed for the patrol
    GetCharacterMovement()->MaxWalkSpeed = 200.0f;
}

void APatrolEnemy::BeginPlay()
{
    Super::BeginPlay();

    // Make sure patrol points have been assigned
    if (PatrolPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("PatrolEnemy: No patrol points assigned!"));
        return;
    }

	UE_LOG(LogTemp, Warning, TEXT("Patrol points assigned: %d"), PatrolPoints.Num());

    // Start moving toward the first patrol point
    MoveToPatrolPoint();

    //Check progress every 0.25 seconds
        GetWorldTimerManager().SetTimer(
        PatrolTimerHandle,
        this,
        &APatrolEnemy::CheckPatrolProgress,
        0.25f,
        true
	);
}

void APatrolEnemy::MoveToPatrolPoint()
{
    if (!PatrolPoints.IsValidIndex(CurrentPatrolIndex))
    {
        return;
    }

    AActor* TargetPoint = PatrolPoints[CurrentPatrolIndex];

    if (!IsValid(TargetPoint))
    {
        UE_LOG(LogTemp, Warning, TEXT("PatrolEnemy: Invalid patrol point!"));
        return;
    }

    AAIController* AIController = Cast<AAIController>(GetController());

    if (!AIController)
    {
        UE_LOG(LogTemp, Error, TEXT("PatrolEnemy: No AI Controller found!"));
        return;
    }

    //tell ai to move toward the next target
    AIController->MoveToLocation(
        TargetPoint->GetActorLocation(),
        PatrolAcceptanceRadius,
        false  // Do not add the character's collision radius to the stopping distance
    );
}


void APatrolEnemy::CheckPatrolProgress()
{
    if (!PatrolPoints.IsValidIndex(CurrentPatrolIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid patrol index: %d"), CurrentPatrolIndex);
        return;
    }

    AActor* TargetPoint = PatrolPoints[CurrentPatrolIndex];

    if (!IsValid(TargetPoint))
    {
        UE_LOG(LogTemp, Error, TEXT("Current patrol point is invalid!"));
        return;
    }

    float DistanceToTarget = FVector::Dist2D(
        GetActorLocation(),
        TargetPoint->GetActorLocation()
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Patrol Index: %d | Distance to target: %.2f"),
        CurrentPatrolIndex,
        DistanceToTarget
    );

    if (DistanceToTarget <= PatrolAcceptanceRadius + 10.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Reached patrol point %d!"), CurrentPatrolIndex);

        MoveToNextPatrolPoint();
    }
}

void APatrolEnemy::MoveToNextPatrolPoint()
{
    CurrentPatrolIndex++;

    if (CurrentPatrolIndex >= PatrolPoints.Num())
    {
        CurrentPatrolIndex = 0;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Moving to next patrol point: %d"),
        CurrentPatrolIndex
    );

    MoveToPatrolPoint();
}


