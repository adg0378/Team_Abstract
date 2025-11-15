// Copyright (c) 2025 Synetos Aerospace


#include "SPEnvConstants.h"

FString USPEnvConstants::GetUserSavePath() {
	static FString UserSavePath = FPaths::ProjectUserDir() / TEXT("SkyPlexUserData");
	return UserSavePath;
}

FString USPEnvConstants::GetCameraLocationPath() {
	static FString CameraLocationPath = GetUserSavePath() / TEXT("cameraLocation.json");
	return CameraLocationPath;
}

FString USPEnvConstants::GetPreferencesPath() {
	static FString PreferencesPath = GetUserSavePath() / TEXT("preferences.json");
	return PreferencesPath;
}
