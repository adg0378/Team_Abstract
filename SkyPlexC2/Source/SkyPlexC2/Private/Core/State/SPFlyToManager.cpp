// Copyright (c) 2025 Synetos Aerospace


#include "Core/State/SPFlyToManager.h"
#include "Util/FileUtility.h"
#include "JsonObjectConverter.h"
#include "Util/SPGeoUtility.h"
#include "SPEnvConstants.h"

USPFlyToManager::USPFlyToManager() {}

void USPFlyToManager::Setup_Implementation() {
	Super::Setup_Implementation();
	if (FPaths::FileExists(USPEnvConstants::GetFlyToLocationPath())) {
		LoadLocations();
	}
	else {
		if (!UFileUtility::WriteStringToFile(*USPEnvConstants::GetFlyToLocationPath(), TEXT(""))) {
			LOG->Error(TEXT("Failed to create fly to location file"), GetClass()->GetName());
		}
	}
}

void USPFlyToManager::GetLocations(TArray<FFlyToLocationData>& outLocations) const {
	outLocations = locations;
}

void USPFlyToManager::DeleteLocation(FFlyToLocationData location) {
	locations.Remove(location);
	WriteLocations();
}

void USPFlyToManager::AddLocationFromCoords(FVector longLatHeight) {
	USPGeoUtility::CoordinatesToAddress(longLatHeight.Y, longLatHeight.X)
		.Then([this, longLatHeight](TFuture<FCoordinatesToAddressResult> resFut) {
		FCoordinatesToAddressResult result = resFut.Get();
		FFlyToLocationData locationResult{ .name = TEXT("Unnamed location"), .longLatHeight = longLatHeight };

		if (result.didFail) {
			LOG->Warn(TEXT("Unable to get address for current location"), GetClass()->GetName());
		}
		else {
			locationResult.name = result.address;
		}

		AddLocation(locationResult);

		AsyncTask(ENamedThreads::GameThread, [this, locationResult]() {
			OnLocationAdded.Broadcast(locationResult);
			});
		});
}

void USPFlyToManager::AddLocation(FFlyToLocationData location) {
	locations.Add(location);
	WriteLocations();
	LOG->Info(FString::Format(*FString("Saved location: '{0}'"), { *location.name }), GetClass()->GetName());
}

void USPFlyToManager::RenameLocation(FFlyToLocationData location, FString newName, FFlyToLocationData& outLocation) {
	for (FFlyToLocationData& loc : locations) {
		if (loc == location) {
			loc.name = newName;
			outLocation = loc;
			LOG->Info(TEXT("Renamed location"), GetClass()->GetName());
			break;
		}
	}
	WriteLocations();
}

void USPFlyToManager::WriteLocations() {

	FFlyToLocations fmtLocations{ .locations = locations };
	FString jsonString;

	if (!FJsonObjectConverter::UStructToJsonObjectString<FFlyToLocations>(fmtLocations, jsonString)) {
		LOG->Error(TEXT("Failed to stringify fly to locations"), GetClass()->GetName());
	}


	if (!UFileUtility::WriteStringToFile(*USPEnvConstants::GetFlyToLocationPath(), *jsonString)) {
		LOG->Error(TEXT("Failed to save fly to locations"), GetClass()->GetName());
	}
}

void USPFlyToManager::LoadLocations() {

	FString fileContents;

	if (!FFileHelper::LoadFileToString(fileContents, *USPEnvConstants::GetFlyToLocationPath())) {
		LOG->Error(TEXT("Failed to load fly to locations"), GetClass()->GetName());
	}

	FFlyToLocations parsedLocations;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(fileContents, &parsedLocations, 0, 0)) {
		LOG->Error(TEXT("Failed to parse fly to locations"), GetClass()->GetName());
	}

	locations = parsedLocations.locations;
}


void USPFlyToManager::UpdateLocationOfLocation(FFlyToLocationData location, FVector newLongLatHeight, FFlyToLocationData& outLocation) {
	for (FFlyToLocationData& loc : locations) {
		if (loc == location) {
			loc.longLatHeight = newLongLatHeight;
			outLocation = loc;
			LOG->Info(TEXT("Updated location"), GetClass()->GetName());
			break;
		}
	}
	WriteLocations();
}

void USPFlyToManager::GetAddressPredictions(FString AddressSearchString) {
	++LatestAddressPredictionRequestID;
	const uint16 ThisRequestID = LatestAddressPredictionRequestID;

	USPGeoUtility::AddressPredictions(AddressSearchString)
		.Then([this, ThisRequestID](TFuture<FGoogleAPIAddressPredictionResult> ResFut) {
		if (ThisRequestID != LatestAddressPredictionRequestID) {
			return;
		}

		const FGoogleAPIAddressPredictionResult& Result = ResFut.Get();
		AsyncTask(ENamedThreads::GameThread, [this, Result]() {
			OnAddressPredictionRecieved.Broadcast(Result);
			});
			});
}

void USPFlyToManager::GetPlaceDetails(FString PlaceID) {
	LatestAddressPredictionRequestID = 0;

	USPGeoUtility::PlaceDetails(PlaceID)
		.Then([this](TFuture<FGooglePlacesResult> ResFut) {
		const FGooglePlacesResult& Result = ResFut.Get();
		if (Result.location.latitude == -999.0 || Result.location.longitude == -999.0) {
			UE_LOG(LogTemp, Error, TEXT("Error retrieving place details"))
		}
		AsyncTask(ENamedThreads::GameThread, [this, Result]() {
			OnPlaceDetailsRecieved.Broadcast(Result);
			});
			});
}

void USPFlyToManager::EndAddressPredictionSession() {
	USPGeoUtility::ClearAddressPredictionSession();
	LatestAddressPredictionRequestID = 0;
}
