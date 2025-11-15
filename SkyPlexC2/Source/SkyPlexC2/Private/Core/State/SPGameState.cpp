// Copyright (c) 2025 Synetos Aerospace

// Must add this if importing Cesium3DTileset.h
#define NOMINMAX
#include "Core/State/SPGameState.h"
#include "Cesium3DTileset.h"
#include "CesiumGeoreference.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"


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
}