// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterAIController.h"
#include "ShooterNPC.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/Navigation/PathFollowingAgentInterface.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h" // FAIStimulus
#include "TimerManager.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogShooterAI, Log, All);

AShooterAIController::AShooterAIController()
{
	// Intentionally simplified: no StateTree or Perception components created by default here.
	// This AIController will only issue a simple MoveToActor request to the player when possessing a pawn.
}

void AShooterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// ensure we're possessing an NPC
	if (AShooterNPC* NPC = Cast<AShooterNPC>(InPawn))
	{
		// add the team tag to the pawn
		NPC->Tags.Add(TeamTag);

		// subscribe to the pawn's OnDeath delegate
		NPC->OnPawnDeath.AddDynamic(this, &AShooterAIController::OnPawnDeath);

		// Try to move to the player. Use a retry timer in case NavMesh / pathfinding is not yet ready.
		TryMoveToPlayer();
	}
}

void AShooterAIController::TryMoveToPlayer()
{
	// Clear any existing retry timer first
	GetWorldTimerManager().ClearTimer(MoveRetryTimer);

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	APawn* MyPawn = GetPawn();

	if (!MyPawn)
	{
		UE_LOG(LogShooterAI, Warning, TEXT("TryMoveToPlayer: controller has no pawn yet; retrying"));
		// Retry until possessed pawn exists
		GetWorldTimerManager().SetTimer(MoveRetryTimer, this, &AShooterAIController::TryMoveToPlayer, MoveRetryInterval, false);
		return;
	}

	if (!PlayerPawn)
	{
		UE_LOG(LogShooterAI, Warning, TEXT("TryMoveToPlayer: no player pawn found; aborting"));
		return;
	}

	// Quick navmesh check: if there's no NavMesh, the request will fail. Use UNavigationSystemV1 to test.
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		UE_LOG(LogShooterAI, Warning, TEXT("TryMoveToPlayer: No NavigationSystem available; retrying"));
		GetWorldTimerManager().SetTimer(MoveRetryTimer, this, &AShooterAIController::TryMoveToPlayer, MoveRetryInterval, false);
		return;
	}

	// Issue the move request and check result
	const EPathFollowingRequestResult::Type Result = MoveToActor(PlayerPawn, 50.0f, true, true, true, 0, true);

	if (Result == EPathFollowingRequestResult::RequestSuccessful || Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		UE_LOG(LogShooterAI, Log, TEXT("TryMoveToPlayer: MoveToActor succeeded (or already at goal)."));
		// success — nothing else to do; ensure no retry timer is active
		GetWorldTimerManager().ClearTimer(MoveRetryTimer);
	}
	else
	{
		UE_LOG(LogShooterAI, Warning, TEXT("TryMoveToPlayer: MoveToActor failed — will retry in %.2f s"), MoveRetryInterval);
		// Retry after a short delay
		GetWorldTimerManager().SetTimer(MoveRetryTimer, this, &AShooterAIController::TryMoveToPlayer, MoveRetryInterval, false);
	}
}

void AShooterAIController::OnPawnDeath()
{
	// stop movement
	if (UPathFollowingComponent* PFC = GetPathFollowingComponent())
	{
		PFC->AbortMove(*this, FPathFollowingResultFlags::UserAbort);
	}

	// cancel any move retry attempts
	GetWorldTimerManager().ClearTimer(MoveRetryTimer);

	// unpossess the pawn
	UnPossess();

	// destroy this controller
	Destroy();
}

void AShooterAIController::SetCurrentTarget(AActor* Target)
{
	TargetEnemy = Target;
}

void AShooterAIController::ClearCurrentTarget()
{
	TargetEnemy = nullptr;
}

void AShooterAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// no-op in simplified setup
}

void AShooterAIController::OnPerceptionForgotten(AActor* Actor)
{
	// no-op in simplified setup
}