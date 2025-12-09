// Copyright (c) 2025 Synetos Aerospace


#include "State/Storage/GeoJsonManager.h"
#include "FileUtility.h"
#include "JsonObjectConverter.h"

UGeoJsonManager::UGeoJsonManager() {
}

void UGeoJsonManager::Setup_Implementation(bool& outSuccess) {
	Super::Setup_Implementation(outSuccess);
	/*if (FPaths::FileExists(GEO_JSON_FILE_PATH)) {*/
	//	//LoadGeoJsons();
	//}
	/*else {
		if (!FileUtility::WriteStringToFile(*GEO_JSON_FILE_PATH, TEXT(""))) {
			LOG->Error(TEXT("Failed to create GeoJson file"), LOG_ORIGIN);
		}
	}*/
}

void UGeoJsonManager::WriteGeoJsons() {
	FGeoJsons geoJsons;
	geoJsons.GeoJsons = GeoJsonArray;
	TArray<TSharedPtr<FJsonValue>> featureCollections;

	for (size_t i = 0; i < geoJsons.GeoJsons.Num(); i++) {
		TSharedPtr<FJsonObject> properties = MakeShared<FJsonObject>();
		properties->SetStringField(TEXT("name"), geoJsons.GeoJsons[i].features.geometry.properties.name);
		properties->SetStringField(TEXT("interestType"), geoJsons.GeoJsons[i].features.geometry.properties.interestType);
		properties->SetNumberField(TEXT("speedAdjustment"), geoJsons.GeoJsons[i].features.geometry.properties.speedAdjustment);
		properties->SetNumberField(TEXT("groupID"), geoJsons.GeoJsons[i].features.geometry.properties.groupID);
		properties->SetNumberField(TEXT("interestID"), geoJsons.GeoJsons[i].features.geometry.properties.interestID);

		TSharedPtr<FJsonObject> geometry = MakeShared<FJsonObject>();
		geometry->SetStringField(TEXT("type"), geoJsons.GeoJsons[i].features.geometry.type);

		TArray<TSharedPtr<FJsonValue>> coordinatesArray;
		coordinatesArray = VectorsToJsonCoords(geoJsons.GeoJsons[i].features.geometry.coordinates);
		geometry->SetArrayField(TEXT("coordinates"), coordinatesArray);
		geometry->SetObjectField(TEXT("properties"), properties);

		TSharedPtr<FJsonObject> feature = MakeShared<FJsonObject>();
		feature->SetStringField(TEXT("type"), geoJsons.GeoJsons[i].features.type);
		feature->SetObjectField(TEXT("geometry"), geometry);

		TSharedPtr<FJsonObject> featureCollection = MakeShared<FJsonObject>();
		featureCollection->SetStringField(TEXT("type"), geoJsons.GeoJsons[i].type);
		featureCollection->SetObjectField(TEXT("features"), feature);

		featureCollections.Add(MakeShared<FJsonValueObject>(featureCollection));
	}

	TSharedPtr<FJsonObject> geoJsonRoot = MakeShared<FJsonObject>();
	FString jsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&jsonString);
	geoJsonRoot->SetArrayField(TEXT("geoJsons"), featureCollections);

	if (FJsonSerializer::Serialize(geoJsonRoot.ToSharedRef(), Writer)) {
		
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed to serialize GeoJsons"))
	}
	
	/*if (!FileUtility::WriteStringToFile(*GEO_JSON_FILE_PATH, *jsonString)) {
		LOG->Error(TEXT("Failed to save GeoJsons"), LOG_ORIGIN);
	}*/

}

bool UGeoJsonManager::LoadGeoJsonsViaFile(FString filePath) {
	FString fileContents;
	FGeoJsons geoJsons;

	if (!FFileHelper::LoadFileToString(fileContents, *filePath)) {
		LOG->Error(TEXT("Failed to load GeoJson file"), LOG_ORIGIN);
		return false;
	}

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*fileContents);
	TSharedPtr<FJsonObject> geoJsonRoot = MakeShared<FJsonObject>();

	if (FJsonSerializer::Deserialize(Reader, geoJsonRoot)) {

	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed to deserialize GeoJsons"));
		return false;
	}

	for (size_t i = 0; i < geoJsonRoot->GetArrayField(TEXT("geoJsons")).Num(); i++) {
		FGeoProperties geoJsonPropterties;
		geoJsonPropterties.name = geoJsonRoot->GetArrayField(TEXT("geoJsons"))[i].Get()->AsObject()->
			GetObjectField(TEXT("features"))->GetObjectField(TEXT("geometry"))->
			GetObjectField(TEXT("properties"))->GetStringField(TEXT("name"));
		geoJsonPropterties.interestType = geoJsonRoot->GetArrayField(TEXT("geoJsons"))[i].Get()->AsObject()->
			GetObjectField(TEXT("features"))->GetObjectField(TEXT("geometry"))->
			GetObjectField(TEXT("properties"))->GetStringField(TEXT("interestType"));
		geoJsonPropterties.speedAdjustment = geoJsonRoot->GetArrayField(TEXT("geoJsons"))[i].Get()->AsObject()->
			GetObjectField(TEXT("features"))->GetObjectField(TEXT("geometry"))->
			GetObjectField(TEXT("properties"))->GetNumberField(TEXT("speedAdjustment"));
		geoJsonPropterties.groupID = geoJsonRoot->GetArrayField(TEXT("geoJsons"))[i].Get()->AsObject()->
			GetObjectField(TEXT("features"))->GetObjectField(TEXT("geometry"))->
			GetObjectField(TEXT("properties"))->GetNumberField(TEXT("groupID"));
		geoJsonPropterties.interestID = geoJsonRoot->GetArrayField(TEXT("geoJsons"))[i].Get()->AsObject()->
			GetObjectField(TEXT("features"))->GetObjectField(TEXT("geometry"))->
			GetObjectField(TEXT("properties"))->GetNumberField(TEXT("interestID"));

		FGeoGeometry geoJsonGrometry;
		geoJsonGrometry.type = geoJsonRoot->GetArrayField(TEXT("geoJsons"))[i].Get()->AsObject()->
			GetObjectField(TEXT("features"))->GetObjectField(TEXT("geometry"))->
			GetStringField(TEXT("type"));

		if (geoJsonGrometry.type.Equals(TEXT("Polygon"))) {
			geoJsonGrometry.coordinates = JsonCoordsArrayToVectors(geoJsonRoot->GetArrayField(TEXT("geoJsons"))[i].Get()->
				AsObject()->GetObjectField(TEXT("features"))->GetObjectField(TEXT("geometry"))->
				GetArrayField(TEXT("coordinates")));

		} else if (geoJsonGrometry.type.Equals(TEXT("Point"))) {
			geoJsonGrometry.coordinates = JsonCoordsToVector(geoJsonRoot->GetArrayField(TEXT("geoJsons"))[i].Get()->
				AsObject()->GetObjectField(TEXT("features"))->GetObjectField(TEXT("geometry"))->
				GetArrayField(TEXT("coordinates")));
		}

		geoJsonGrometry.properties = geoJsonPropterties;

		FGeoFeatures geoJsonFeatures;
		geoJsonFeatures.type = geoJsonRoot->GetArrayField(TEXT("geoJsons"))[i].Get()->AsObject()->
			GetObjectField(TEXT("features"))->GetStringField(TEXT("type"));
		geoJsonFeatures.geometry = geoJsonGrometry;

		FGeoJsonData geoJsonData;
		geoJsonData.type = geoJsonRoot->GetArrayField(TEXT("geoJsons"))[i].Get()->AsObject()->GetStringField(TEXT("type"));
		geoJsonData.features = geoJsonFeatures;

		geoJsons.GeoJsons.Add(geoJsonData);
	}

	GeoJsonArray = geoJsons.GeoJsons;
	return true;
}

void UGeoJsonManager::AddGeoJson(FGeoJsonData geoJson) {
	GeoJsonArray.Add(geoJson);
	WriteGeoJsons();
	LOG->Info(FString::Format(*FString("Saved GeoJson: '{0}'"), { *geoJson.features.geometry.properties.name }), LOG_ORIGIN);
}


void UGeoJsonManager::GetGeoJsons(TArray<FGeoJsonData>& outGeoJsons) const {
	outGeoJsons = GeoJsonArray;
}

TArray<TSharedPtr<FJsonValue>> UGeoJsonManager::VectorsToJsonCoords(TArray<FVector> vectors) {
	TArray<TSharedPtr<FJsonValue>> returnJsonArray;
		
	if (vectors.Num() == 1) {
		returnJsonArray.Add(MakeShared<FJsonValueNumber>(vectors[0].X));
		returnJsonArray.Add(MakeShared<FJsonValueNumber>(vectors[0].X));
		return returnJsonArray;
	}

	for (const FVector& coords : vectors){
		TArray<TSharedPtr<FJsonValue>> coordinates{
			MakeShared<FJsonValueNumber>(coords.X),
			MakeShared<FJsonValueNumber>(coords.Y),
		};

		returnJsonArray.Add(MakeShared<FJsonValueArray>(coordinates));
	}

	return returnJsonArray;
}

TArray<FVector> UGeoJsonManager::JsonCoordsArrayToVectors(TArray<TSharedPtr<FJsonValue>> geoJsonCoordsArray) {
	TArray<FVector> returnVectorArray;

	for (size_t i = 0; i < geoJsonCoordsArray.Num(); i++){
		FVector coordinates;
		coordinates.X = geoJsonCoordsArray[i].Get()->AsArray()[0].Get()->AsNumber();
		coordinates.Y = geoJsonCoordsArray[i].Get()->AsArray()[1].Get()->AsNumber();
		coordinates.Z = 0.0;
		returnVectorArray.Add(coordinates);
	}

	return returnVectorArray;
}

TArray<FVector> UGeoJsonManager::JsonCoordsToVector(TArray<TSharedPtr<FJsonValue>> geoJsonCoords) {
	TArray<FVector> returnVectorArray;

	FVector coordinates;
	coordinates.X = geoJsonCoords[0].Get()->AsNumber();
	coordinates.Y = geoJsonCoords[1].Get()->AsNumber();
	coordinates.Z = 0.0;
	returnVectorArray.Add(coordinates);

	return returnVectorArray;
}

void UGeoJsonManager::ClearGeoJsons() {
	GeoJsonArray.Empty();
	WriteGeoJsons();
}

void UGeoJsonManager::AddGeoJsonPolygon(FString name, FString interestType, int32 groupID, int32 interestID, TArray<FVector> coords) {
	FGeoProperties geoJsonPropterties;
	geoJsonPropterties.name = name;
	geoJsonPropterties.interestType = interestType;
	geoJsonPropterties.speedAdjustment = 1;
	geoJsonPropterties.groupID = groupID;
	geoJsonPropterties.interestID = interestID;

	FGeoGeometry geoJsonGrometry;
	geoJsonGrometry.type = TEXT("Polygon");
	geoJsonGrometry.coordinates = coords;
	geoJsonGrometry.properties = geoJsonPropterties;

	FGeoFeatures geoJsonFeatures;
	geoJsonFeatures.type = (TEXT("Feature"));
	geoJsonFeatures.geometry = geoJsonGrometry;

	FGeoJsonData geoJsonData;
	geoJsonData.type = (TEXT("FeatureCollection"));
	geoJsonData.features = geoJsonFeatures;

	GeoJsonArray.Add(geoJsonData);
	WriteGeoJsons();
	LOG->Info(FString::Format(*FString("Saved GeoJsonPolygon: '{0}'"), { *name }), LOG_ORIGIN);

}

void UGeoJsonManager::AddGeoJsonPoint(FString name, FString interestType, int32 groupID, int32 interestID, FVector coords) {
	FGeoProperties geoJsonPropterties;
	geoJsonPropterties.name = name;
	geoJsonPropterties.interestType = interestType;
	geoJsonPropterties.speedAdjustment = 1;
	geoJsonPropterties.groupID = groupID;
	geoJsonPropterties.interestID = interestID;

	FGeoGeometry geoJsonGrometry;
	geoJsonGrometry.type = TEXT("Point");
	TArray<FVector> wrapperArray;
	wrapperArray.Add(coords);
	geoJsonGrometry.coordinates = wrapperArray;
	geoJsonGrometry.properties = geoJsonPropterties;

	FGeoFeatures geoJsonFeatures;
	geoJsonFeatures.type = (TEXT("Feature"));
	geoJsonFeatures.geometry = geoJsonGrometry;

	FGeoJsonData geoJsonData;
	geoJsonData.type = (TEXT("FeatureCollection"));
	geoJsonData.features = geoJsonFeatures;

	GeoJsonArray.Add(geoJsonData);
	WriteGeoJsons();
	LOG->Info(FString::Format(*FString("Saved GeoJsonPoint: '{0}'"), { *name }), LOG_ORIGIN);
}

void UGeoJsonManager::RenameGeoJson(FString newName, int32 interestID) {
	for (FGeoJsonData& geoJsonData : GeoJsonArray) {
		if (geoJsonData.features.geometry.properties.interestID == interestID) {
			geoJsonData.features.geometry.properties.name = newName;
			LOG->Info(TEXT("Renamed geoJson"), LOG_ORIGIN);
			break;
		}
	}

	WriteGeoJsons();
}

void UGeoJsonManager::UpdateCoordsGeoJsonPolygon(TArray<FVector> coords, int32 interestID) {
	for (FGeoJsonData& geoJsonData : GeoJsonArray) {
		if (geoJsonData.features.geometry.properties.interestID == interestID) {
			geoJsonData.features.geometry.coordinates = coords;
			LOG->Info(TEXT("Updated geoJson Polygon coords"), LOG_ORIGIN);
			break;
		}
	}

	WriteGeoJsons();
}

void UGeoJsonManager::UpdateCoordsGeoJsonPoint(FVector coords, int32 interestID) {
	for (FGeoJsonData& geoJsonData : GeoJsonArray) {
		if (geoJsonData.features.geometry.properties.interestID == interestID) {
			TArray<FVector> wrapperArray;
			wrapperArray.Add(coords);
			geoJsonData.features.geometry.coordinates = wrapperArray;
			LOG->Info(TEXT("Updated geoJson Point coords"), LOG_ORIGIN);
			break;
		}
	}

	WriteGeoJsons();
}

void UGeoJsonManager::DeleteGeoJson(int32 interestID) {
	for (FGeoJsonData& geoJsonData : GeoJsonArray) {
		if (geoJsonData.features.geometry.properties.interestID == interestID) {
			GeoJsonArray.RemoveSingle(geoJsonData);
			LOG->Info(FString::Format(*FString("Delete GeoJson with ID: '{0}'"), { interestID }), LOG_ORIGIN);
			break;
		}
	}

	WriteGeoJsons();
}
