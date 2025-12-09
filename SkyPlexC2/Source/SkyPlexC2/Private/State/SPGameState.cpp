#include "State/SPGameState.h"
#include "Player/SPPlayerController.h"
#include "State/PreferencesManager.h"
#include "State/SPWorldManager.h"
#include "State/SPAuthManager.h"
#include "State/Missions/SPMissionManager.h"
#include "State/Storage/SPStorageManager.h"
#include "State/Storage/SPSimulationManager.h"
#include "State/Obstacles/SPObstacleManager.h"
#include "SPDroneManager.h"
#include "Objects/Interactable.h"

#undef GetObject

void ASPGameState::BeginPlay() {
	Super::BeginPlay();
	bool outSuccess = true;

	// non-manager initialization

	SPWorld = GetWorld();
	FString LogOrigin = TEXT("SPGameState");

	if (!SPWorld) {
		loggerRef->Error(TEXT("Failed to initialize: no World"));
		return;
	}

	CesiumGeoreference = ACesiumGeoreference::GetDefaultGeoreference(SPWorld);

	for (TActorIterator<ACesium3DTileset> It(SPWorld); It; ++It) {
		ACesium3DTileset* Tileset = *It;
		if (Tileset && Tileset->IsActorInitialized()) {
			CesiumTileset = Tileset;
		}
	}
	if (!CesiumTileset) {
		loggerRef->Error(TEXT("Failed to initialize: no ACesium3DTileset"));
		return;
	}

	// SETUP

	PreferencesManager = NewObject<UPreferencesManager>(this, PreferencesManagerToSpawn);
	PreferencesManager->Setup();

	loggerRef = NewObject<USPLogger>(this, LoggerToSpawn);
	
	storageManagerRef = NewObject<USPStorageManager>(this, StorageManagerToSpawn);
	storageManagerRef->Setup(outSuccess);

	AuthManagerRef = NewObject<USPAuthManager>(this, AuthManagerToSpawn);
	AuthManagerRef->Setup(outSuccess);

	WorldManagerRef = NewObject<USPWorldManager>(this, WorldManagerToSpawn);
	WorldManagerRef->Setup(outSuccess);

	MissionManagerRef = NewObject<USPMissionManager>(this, MissionManagerToSpawn);
	MissionManagerRef->Setup(outSuccess);

	obstacleManagerRef = NewObject<USPObstacleManager>(this, ObstacleManagerToSpawn);
	obstacleManagerRef->Setup(outSuccess);

	droneManagerRef = NewObject<USPDroneManager>(this, DroneManagerToSpawn);
	droneManagerRef->Setup(outSuccess);

	simulationManagerRef = NewObject<USPSimulationManager>(this, SimulationManagerToSpawn);
	simulationManagerRef->Setup(outSuccess);
	// POST SETUP

	WorldManagerRef->PostSetup();

	FullyInitialized = true;
	OnGameStateFullyInitialized.Broadcast();

	loggerRef->Info(TEXT("GameState initialized."), LogOrigin);
}

bool ASPGameState::IsFullyInitialized() const {
	return FullyInitialized;
}

void ASPGameState::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	// PRE-TEARDOWN
	MissionManagerRef->PreTeardown();
	WorldManagerRef->PreTeardown();

	// TEARDOWN
	droneManagerRef->Teardown();
	obstacleManagerRef->Teardown();

	Super::EndPlay(EndPlayReason);
}

void ASPGameState::AddSelected(AInteractable* actor) {
	ASPPlayerController* controller = USPUtility::GetSPPlayerController(this);
	if (controller->mode != EControllerMode::MultiSelect) {
		DeselectAll();
	}
	int index = selectedActors.Add(actor);
	if (loggerRef) {
		loggerRef->Debug(FString("Selected"));
	}

	RefreshSelected();
}

void ASPGameState::RemoveSelected(AInteractable* actor) {
	selectedActors.Remove(actor);
	if (loggerRef) {
		loggerRef->Debug(FString("Deselected"));
	}

	RefreshSelected();
}

void ASPGameState::RefreshSelected() {
	if (selectedActors.IsEmpty()) {
		DeleteInteractionBox();
	}
	else {
		FText title;
		CompileInteractionBoxTitle(title);
		TMap<FString, FInteractionBoxValue> keyVals;
		CompileInteractionBoxKeyVals(keyVals);
		UpdateInteractionBox(title, keyVals);
	}
}

void ASPGameState::DeselectAll() {
	for (AInteractable* actor : selectedActors) {
		actor->Deselect(false);
	}
	selectedActors.Empty();
	if (loggerRef) {
		loggerRef->Debug(FString("Deselected all actors"));
	}

	DeleteInteractionBox();
}

void ASPGameState::IsSelected(bool& isSelected) const {
	isSelected = !selectedActors.IsEmpty();
}

void ASPGameState::IsMultipleSelected(bool& isMultipleSelected) const {
	isMultipleSelected = selectedActors.Num() > 1;
}

void ASPGameState::DeleteSelected() {
	TArray<AInteractable*> SelectedCopy = selectedActors;
	for (AInteractable* actor : SelectedCopy) {
		if (actor->userPlacementEnabled) {
			IInteractionBoxProvider::Execute_DestroySelf(actor);
		}
	}

	DeleteInteractionBox();
}

void ASPGameState::UpdateInteractionBox_Implementation(const FText& title, const TMap<FString, FInteractionBoxValue>& keyVals) {

}

void ASPGameState::DeleteInteractionBox_Implementation() {

}

void ASPGameState::CompileInteractionBoxKeyVals(TMap<FString, FInteractionBoxValue>& outKeyVals) {
	outKeyVals.Empty();

	if (selectedActors.Num() == 1) {
		IInteractionBoxProvider::Execute_GetInteractionBoxKeyVals(selectedActors[0], outKeyVals);
		return;
	}

	bool InitializedKeyVals = false;
	for (AInteractable* actor : selectedActors) {
		TMap<FString, FInteractionBoxValue> actorKeyVals;
		IInteractionBoxProvider::Execute_GetInteractionBoxKeyVals(actor, actorKeyVals);

		if (!InitializedKeyVals) {
			outKeyVals = actorKeyVals;
			InitializedKeyVals = true;
			continue;
		}

		TArray<FString> keys;
		outKeyVals.GetKeys(keys);

		for (const FString& key : keys) {
			if (!actorKeyVals.Contains(key) || !(*actorKeyVals.Find(key) == *outKeyVals.Find(key))) {
				outKeyVals.Remove(key);
			}
		}
	}
}

void ASPGameState::CompileInteractionBoxTitle(FText& outTitle) {
	if (selectedActors.Num() == 1) {
		IInteractionBoxProvider::Execute_GetInteractionBoxTitle(selectedActors[0], outTitle);
		return;
	}

	outTitle = FText::FromString(TEXT("Multiple Selected"));
}

void ASPGameState::SampleHeights(const TArray<FVector>& LonLatHeightArray, FSPSampleHeightCallback Callback) {
	UWorld* World = GetWorld();
	FString LogOrigin = TEXT("SampleHeights");

	if (!World) {
		loggerRef->Error(TEXT("Failed to sample heights: No world context"), LogOrigin);
		const TArray<FVector> Result;
		Callback(Result);
	}

	FCesiumSampleHeightMostDetailedCallback CesiumCallback = FCesiumSampleHeightMostDetailedCallback::CreateLambda(
		[Callback, this, LogOrigin](ACesium3DTileset* Tileset, const TArray<FCesiumSampleHeightResult>& Results, const TArray<FString>& Warnings) {
			TArray<FVector> SampledPositions;
			SampledPositions.Reserve(Results.Num());

			for (const FString& Warning : Warnings) {
				loggerRef->Warn(Warning, LogOrigin);
			}

			for (const FCesiumSampleHeightResult& Result : Results) {
				const FVector& InputPos = Result.LongitudeLatitudeHeight;
				if (Result.SampleSuccess) {
					SampledPositions.Add(FVector(InputPos.X, InputPos.Y, InputPos.Z));
				}
				else {
					SampledPositions.Add(FVector(InputPos.X, InputPos.Y, -9999.0));
				}
			}

			Callback(SampledPositions);
		}
	);

	CesiumTileset->SampleHeightMostDetailed(LonLatHeightArray, CesiumCallback);
}

void ASPGameState::SwapActivePlayerCamera_Implementation(AActor* InCamera) {

}

void ASPGameState::GetSelectedTopLevelProviders(TArray<UObject*>& Providers) const {
	for (auto& Interactable : selectedActors) {
		if (!Interactable->GetClass()->ImplementsInterface(UInteractionBoxProvider::StaticClass()))
		{
			UE_LOG(LogTemp, Error, TEXT("%s does not implement InteractionBoxProvider"), *Interactable->GetName());
			continue;
		}

		TScriptInterface<UInteractionBoxProvider> Current;
		IInteractionBoxProvider::Execute_GetLinkedProvider(Interactable, Current);

		if (!Current.GetObject()) {
			/*UE_LOG(LogTemp, Warning, TEXT("%s has no immediate linked provider"), *Interactable->GetName());*/
			continue;
		}


		while (true) {
			TScriptInterface<UInteractionBoxProvider> Next;
			IInteractionBoxProvider::Execute_GetLinkedProvider(Current.GetObject(), Next);

			if (!Next.GetObject()) {
				break;
			}
			Current = Next;
		}

		Providers.Add(Current.GetObject());
	}
}
