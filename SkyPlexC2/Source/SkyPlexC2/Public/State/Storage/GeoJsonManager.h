// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "State/SPManager.h"
#include "GeoJsonManager.generated.h"


/**
 * GeoJsonPropterties struct stores: name, interestType, and speed adjustment
 */
USTRUCT(BlueprintType)
struct FGeoProperties {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString interestType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float speedAdjustment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 groupID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 interestID;

	bool operator==(const FGeoProperties& other) const
	{
		return interestType == other.interestType && speedAdjustment == other.speedAdjustment && 
			name == other.name &&  groupID == other.groupID && interestID == other.interestID;
	}
};

/**
 * GeoJsonGeometry struct stores: (geometry)type, FGeoCoordinates, and custom Geo FProperties struct
 */
USTRUCT(BlueprintType)
struct FGeoGeometry {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> coordinates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGeoProperties properties;

	bool operator==(const FGeoGeometry& other) const
	{
		return type == other.type && coordinates == other.coordinates && properties == other.properties;
	}
};

/**
 * GeoJsonFeatures struct stores: (Feature)type, and Geo FGeometry struct
 */
USTRUCT(BlueprintType)
struct FGeoFeatures {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGeoGeometry geometry;

	bool operator==(const FGeoFeatures& other) const
	{
		return type == other.type && geometry == other.geometry;
	}
};

/**
 * GeoJson Data Struct for interests, stores: (FeatureCollection)type, and Geo FFeatures struct;
 */
USTRUCT(BlueprintType)
struct FGeoJsonData {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGeoFeatures features;

	bool operator==(const FGeoJsonData& other) const
	{
		return type == other.type && features == other.features;
	}
};

USTRUCT(BlueprintType)
struct FGeoJsons {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FGeoJsonData> GeoJsons;
};

UCLASS()
class SKYPLEXC2_API UGeoJsonManager : public USPManager {
	GENERATED_BODY()
	
public:
	UGeoJsonManager();

	void Setup_Implementation(bool& outSuccess) override;

	UFUNCTION(BlueprintCallable)
	void WriteGeoJsons();

	bool LoadGeoJsonsViaFile(FString filePath);

	void AddGeoJson(FGeoJsonData geoJson);

	void GetGeoJsons(TArray<FGeoJsonData>& outGeoJsons) const;

	//empties the GeoJsonManager's GeoJsonData Array 
	void ClearGeoJsons();

	void AddGeoJsonPolygon(FString name, FString interestType, int32 groupID, int32 interestID, TArray<FVector> coords);

	void AddGeoJsonPoint(FString name, FString interestType, int32 groupID, int32 interestID, FVector coords);

	void RenameGeoJson(FString newName, int32 interestID);

	void UpdateCoordsGeoJsonPolygon(TArray<FVector> coords, int32 interestID);

	void UpdateCoordsGeoJsonPoint(FVector coords, int32 interestID);

	void DeleteGeoJson(int32 interestID);


private:

	TArray<FGeoJsonData> GeoJsonArray;
	const FString JSON_FILENAME = TEXT("geoJson.json");
	/*const FString GEO_JSON_FILE_PATH = USPEnvConstants::USER_SAVE_PATH / JSON_FILENAME;*/
	const FString LOG_ORIGIN = TEXT("GeoJsonManager");

	//Helper function to correctly format Vectors with lon and lat coords into well formatted GeoJson coords as a JsonValue
	TArray<TSharedPtr<FJsonValue>> VectorsToJsonCoords(TArray<FVector> vectors);

	//Helper function to correctly format GeoJson Polygon coords as FVectors for interests
	TArray<FVector> JsonCoordsArrayToVectors(TArray<TSharedPtr<FJsonValue>> geoJsonCoordsArray);
	
	//Helper function to correctly format GeoJson Point coords as a FVector for interests
	TArray<FVector> JsonCoordsToVector(TArray<TSharedPtr<FJsonValue>> geoJsonCoords);

};
