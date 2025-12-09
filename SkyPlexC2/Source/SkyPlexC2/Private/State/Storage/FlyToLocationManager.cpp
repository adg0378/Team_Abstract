#include "State/Storage/FlyToLocationManager.h"
#include "API/GoogleAPIs.h"
#include "SPUtility.h"
#include "Player/SPPlayerController.h"
#include "FileUtility.h"
#include "JsonObjectConverter.h"

const FString LOG_ORIGIN = TEXT("FlyToLocationManager");

UFlyToLocationManager::UFlyToLocationManager() {
}

void UFlyToLocationManager::Setup_Implementation(bool& outSuccess) {
	Super::Setup_Implementation(outSuccess);
	if (FPaths::FileExists(USPEnvConstants::GetFlyToLocationPath())) {
		LoadLocations();
	}
	else {
		if (!FileUtility::WriteStringToFile(*USPEnvConstants::GetFlyToLocationPath(), TEXT("{}"))) {
			LOG->Error(TEXT("Failed to create fly to location file"), LOG_ORIGIN);
		}
	}
}

void UFlyToLocationManager::GetLocations(TArray<FFlyToLocationData>& outLocations) const {
	outLocations = locations;
}

void UFlyToLocationManager::DeleteLocation(FFlyToLocationData location) {
	locations.Remove(location);
	WriteLocations();
}

void UFlyToLocationManager::AddLocationFromCoords(FVector longLatHeight) {
	UGoogleAPIs::CoordinatesToAddress(LOG, longLatHeight.Y, longLatHeight.X)
		.Then([this, longLatHeight](TFuture<FCoordinatesToAddressResult> resFut) {
			FCoordinatesToAddressResult result = resFut.Get();
			FFlyToLocationData locationResult{ .name = TEXT("Unnamed location"), .longLatHeight = longLatHeight };

			if (result.didFail) {
				LOG->Warn(TEXT("Unable to get address for current location"), LOG_ORIGIN);
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

void UFlyToLocationManager::AddLocation(FFlyToLocationData location) {
	locations.Add(location);
	WriteLocations();
	LOG->Info(FString::Format(*FString("Saved location: '{0}'"), { *location.name }), LOG_ORIGIN);
}

void UFlyToLocationManager::RenameLocation(FFlyToLocationData location, FString newName, FFlyToLocationData& outLocation) {
	for (FFlyToLocationData& loc : locations) {
		if (loc == location) {
			loc.name = newName;
			outLocation = loc;
			LOG->Info(TEXT("Renamed location"), LOG_ORIGIN);
			break;
		}
	}
	WriteLocations();
}

void UFlyToLocationManager::WriteLocations() {

	FFlyToLocations fmtLocations{ .locations = locations };
	FString jsonString;

	if (!FJsonObjectConverter::UStructToJsonObjectString<FFlyToLocations>(fmtLocations, jsonString)) {
		LOG->Error(TEXT("Failed to stringify fly to locations"), LOG_ORIGIN);
	}

	
	if (!FileUtility::WriteStringToFile(*USPEnvConstants::GetFlyToLocationPath(), *jsonString)) {
		LOG->Error(TEXT("Failed to save fly to locations"), LOG_ORIGIN);
	}
}

void UFlyToLocationManager::LoadLocations() {

	FString fileContents;

	if (!FFileHelper::LoadFileToString(fileContents, *USPEnvConstants::GetFlyToLocationPath())) {
		LOG->Error(TEXT("Failed to load fly to locations"), LOG_ORIGIN);
	}

	FFlyToLocations parsedLocations;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(fileContents, &parsedLocations, 0, 0)) {
		LOG->Error(TEXT("Failed to parse fly to locations"), LOG_ORIGIN);
	}

	locations = parsedLocations.locations;
}


void UFlyToLocationManager::UpdateLocationOfLocation(FFlyToLocationData location, FVector newLongLatHeight, FFlyToLocationData& outLocation) {
	for (FFlyToLocationData& loc : locations) {
		if (loc == location) {
			loc.longLatHeight = newLongLatHeight;
			outLocation = loc;
			LOG->Info(TEXT("Updated location"), LOG_ORIGIN);
			break;
		}
	}
	WriteLocations();
}

void UFlyToLocationManager::GetAddressPredictions(FString AddressSearchString) {
	++LatestAddressPredictionRequestID;
	const uint16 ThisRequestID = LatestAddressPredictionRequestID;

	UGoogleAPIs::AddressPredictions(AddressSearchString)
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

void UFlyToLocationManager::GetPlaceDetails(FString PlaceID) {
	LatestAddressPredictionRequestID = 0;

	UGoogleAPIs::PlaceDetails(PlaceID)
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

void UFlyToLocationManager::EndAddressPredictionSession() {
	UGoogleAPIs::ClearAddressPredictionSession();
	LatestAddressPredictionRequestID = 0;
}

