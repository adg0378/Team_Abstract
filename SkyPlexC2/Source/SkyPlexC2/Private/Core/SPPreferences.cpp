// Copyright (c) 2025 Synetos Aerospace


#include "Core/SPPreferences.h"
#include "JsonObjectConverter.h"
#include "Core/SPLogger.h"
#include "Util/FileUtility.h"
#include "SPEnvConstants.h"
#include "Core/State/SPGameState.h"

void USPPreferences::Setup() {
	GameStateRef = ASPGameState::GetSPGameState(this);
	LOG = GameStateRef->Logger;
	ReadPreferences();
}
void USPPreferences::Teardown() {
	WritePreferences();
}

void USPPreferences::WritePreferences() const {
	FString JsonString;

	if (!FJsonObjectConverter::UStructToJsonObjectString<FSPPreferencesStruct>(Preferences, JsonString)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to stringify preferences data"));
	}


	if (!UFileUtility::WriteStringToFile(*USPEnvConstants::GetPreferencesPath(), *JsonString)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to save preferences data"));
	}
}

void USPPreferences::ReadPreferences() {
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

FSPPreferencesStruct USPPreferences::GetPreferences() const {
	return Preferences;
}

const FSPPreferencesStruct& USPPreferences::GetPreferencesRef() const {
	return Preferences;
}

void USPPreferences::SetPreferences(const FSPPreferencesStruct& InPreferences) {
	FSPPreferencesStruct PrevPreferences = Preferences;
	Preferences = InPreferences;
	WritePreferences();
	OnPreferencesUpdated.Broadcast(PrevPreferences, Preferences);
}