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

FString USPEnvConstants::GetFlyToLocationPath() {
	static FString FlyToLocationPath = GetUserSavePath() / TEXT("flyToLocations.json");
	return FlyToLocationPath;
}

FString USPEnvConstants::GetDatabasePath() {
	static FString DatabasePath = GetUserSavePath() / TEXT("skyplexc2.db");
	return DatabasePath;
}

const FString USPEnvConstants::FIREBASE_URL = TEXT("https://identitytoolkit.googleapis.com/v1/");
const FString USPEnvConstants::GOOGLE_TOKEN_URL = TEXT("https://securetoken.googleapis.com/v1/");
