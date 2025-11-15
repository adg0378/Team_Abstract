// Copyright (c) 2025 Synetos Aerospace

// Must add this if importing Cesium3DTileset.h
#define NOMINMAX
#include "Core/State/SPGameState.h"
#include "Cesium3DTileset.h"
#include "CesiumGeoreference.h"
#include "EngineUtils.h"
#include "Core/SPPreferences.h"
#include "Core/SPLogger.h"
#include "Core/State/SPAuthManager.h"
#include "Core/State/SPDatabaseManager.h"
#include "Core/State/SPDroneManager.h"
#include "Core/State/SPFlyToManager.h"
#include "Core/State/SPMissionManager.h"
#include "Core/State/SPObstacleManager.h"
#include "Core/State/SPSelectionManager.h"
#include "Core/State/SPSimulationManager.h"
#include "Core/State/SPWorldManager.h"
#include "Kismet/GameplayStatics.h"

ASPGameState::ASPGameState() {
	LoggerToSpawn = USPLogger::StaticClass();
	PreferencesManagerToSpawn = USPPreferences::StaticClass();
	AuthManagerToSpawn = USPAuthManager::StaticClass();
	WorldManagerToSpawn = USPWorldManager::StaticClass();
	DatabaseManagerToSpawn = USPDatabaseManager::StaticClass();
	FlyToManagerToSpawn = USPFlyToManager::StaticClass();
	DroneManagerToSpawn = USPDroneManager::StaticClass();
	MissionManagerToSpawn = USPMissionManager::StaticClass();
	ObstacleManagerToSpawn = USPObstacleManager::StaticClass();
	SimulationManagerToSpawn = USPSimulationManager::StaticClass();
	SelectionManagerToSpawn = USPSelectionManager::StaticClass();
}

ASPGameState* ASPGameState::GetSPGameState(const UObject* WorldContextObject) {
	UWorld* World = WorldContextObject->GetWorld();
	AGameStateBase* GameState = World ? UGameplayStatics::GetGameState(World) : nullptr;
	ASPGameState* SPGameState = GameState ? Cast<ASPGameState>(GameState) : nullptr;
	if (!SPGameState) {
		UE_LOG(LogTemp, Warning, TEXT("Failed to get SPGameState"));
	}
	return SPGameState;
}

void ASPGameState::BeginPlay() {
	Super::BeginPlay();

	SPWorld = GetWorld();

	CesiumGeoreference = ACesiumGeoreference::GetDefaultGeoreference(SPWorld);

	for (TActorIterator<ACesium3DTileset> It(SPWorld); It; ++It) {
		ACesium3DTileset* Tileset = *It;
		if (Tileset && Tileset->IsActorInitialized()) {
			CesiumTileset = Tileset;
		}
	}
	if (!CesiumTileset) {
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize: no ACesium3DTileset"));
		return;
	}

	// SETUP

	Preferences = NewObject<USPPreferences>(this, PreferencesManagerToSpawn);
	Logger = NewObject<USPLogger>(this, LoggerToSpawn);
	AuthManager = NewObject<USPAuthManager>(this, AuthManagerToSpawn);
	WorldManager = NewObject<USPWorldManager>(this, WorldManagerToSpawn);
	DatabaseManager = NewObject<USPDatabaseManager>(this, DatabaseManagerToSpawn);
	FlyToManager = NewObject<USPFlyToManager>(this, FlyToManagerToSpawn);
	DroneManager = NewObject<USPDroneManager>(this, DroneManagerToSpawn);
	MissionManager = NewObject<USPMissionManager>(this, MissionManagerToSpawn);
	ObstacleManager = NewObject<USPObstacleManager>(this, ObstacleManagerToSpawn);
	SimulationManager = NewObject<USPSimulationManager>(this, SimulationManagerToSpawn);
	SelectionManager = NewObject<USPSelectionManager>(this, SelectionManagerToSpawn);

	Preferences->Setup();
	AuthManager->Setup();
	WorldManager->Setup();
	DatabaseManager->Setup();
	FlyToManager->Setup();
	DroneManager->Setup();
	MissionManager->Setup();
	ObstacleManager->Setup();
	SimulationManager->Setup();
	SelectionManager->Setup();

	// POST SETUP

	AuthManager->PostSetup();
	WorldManager->PostSetup();
	DatabaseManager->PostSetup();
	FlyToManager->PostSetup();
	DroneManager->PostSetup();
	MissionManager->PostSetup();
	ObstacleManager->PostSetup();
	SimulationManager->PostSetup();
	SelectionManager->PostSetup();

	Logger->Info(TEXT("Initialized successfully"));
}

void ASPGameState::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	// PRE-TEARDOWN

	SelectionManager->PreTeardown();
	SimulationManager->PreTeardown();
	ObstacleManager->PreTeardown();
	MissionManager->PreTeardown();
	DroneManager->PreTeardown();
	FlyToManager->PreTeardown();
	DatabaseManager->PreTeardown();
	WorldManager->PreTeardown();
	AuthManager->PreTeardown();

	// TEARDOWN

	SelectionManager->Teardown();
	SimulationManager->Teardown();
	ObstacleManager->Teardown();
	MissionManager->Teardown();
	DroneManager->Teardown();
	FlyToManager->Teardown();
	DatabaseManager->Teardown();
	WorldManager->Teardown();
	AuthManager->Teardown();
	Preferences->Teardown();

	Super::EndPlay(EndPlayReason);
}