// Copyright (c) 2025 Synetos Aerospace


#include "State/PreferencesManager.h"
#include "JsonObjectConverter.h"
#include "SPLogger.h"
#include "State/SPGameState.h"
#include "FileUtility.h"

void UPreferencesManager::Setup() {
	GameStateRef = USPUtility::GetSPGameState(this);
	LOG = GameStateRef->loggerRef;
	ReadPreferences();
}
void UPreferencesManager::Teardown() {
	WritePreferences();
}

void UPreferencesManager::WritePreferences() const {
	FString JsonString;

	if (!FJsonObjectConverter::UStructToJsonObjectString<FSPPreferencesStruct>(Preferences, JsonString)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to stringify faa cache data"));
	}


	if (!FileUtility::WriteStringToFile(*USPEnvConstants::GetPreferencesPath(), *JsonString)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to save faa cache data"));
	}
}

void UPreferencesManager::ReadPreferences() {
	if (FPaths::FileExists(USPEnvConstants::GetPreferencesPath())) {
		FString FileContents;

		if (!FFileHelper::LoadFileToString(FileContents, *USPEnvConstants::GetPreferencesPath())) {
			LOG->Error(TEXT("Failed to load preferences"), LOG_ORIGIN);
			return;
		}

		FSPPreferencesStruct ParsedPreferences;
		if (!FJsonObjectConverter::JsonObjectStringToUStruct<FSPPreferencesStruct>(FileContents, &ParsedPreferences, 0, 0)) {
			LOG->Error(TEXT("Failed to parse preferences"), LOG_ORIGIN);
			return;
		}
		
		Preferences = ParsedPreferences;
	}
}

FSPPreferencesStruct UPreferencesManager::GetPreferences() const {
	return Preferences;
}

const FSPPreferencesStruct& UPreferencesManager::GetPreferencesRef() const {
	return Preferences;
}

void UPreferencesManager::SetPreferences(const FSPPreferencesStruct& InPreferences) {
	FSPPreferencesStruct PrevPreferences = Preferences;
	Preferences = InPreferences;
	WritePreferences();
	OnPreferencesUpdated.Broadcast(PrevPreferences, Preferences);
}