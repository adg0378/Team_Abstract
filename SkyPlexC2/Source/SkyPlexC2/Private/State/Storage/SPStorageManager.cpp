#include "State/Storage/SPStorageManager.h"
#include "State/Storage/FlyToLocationManager.h"
#include "State/Storage/MissionAssetsManager.h"
#include "State/Storage/GeoJsonManager.h"
#include <filesystem>
#include <Misc/CoreDelegates.h>

namespace fs = std::filesystem;


USPStorageManager::USPStorageManager()
{
}

USPStorageManager::~USPStorageManager() {
}

void USPStorageManager::Setup_Implementation(bool& outSuccess) {
	Super::Setup_Implementation(outSuccess);

	// Ensure user save directory is created
	fs::create_directories(TCHAR_TO_UTF8(*USPEnvConstants::GetUserSavePath()));

	UWorld* world = this->GetWorld();
	flyToLocationManagerRef = NewObject<UFlyToLocationManager>(world, UFlyToLocationManager::StaticClass());
	missionAssetsManagerRef = NewObject<UMissionAssetsManager>(world, UMissionAssetsManager::StaticClass());
	geoJsonManagerRef = NewObject<UGeoJsonManager>(world, UGeoJsonManager::StaticClass());

	flyToLocationManagerRef->Setup_Implementation(outSuccess);
	missionAssetsManagerRef->Setup_Implementation(outSuccess);
	geoJsonManagerRef->Setup_Implementation(outSuccess);
}










