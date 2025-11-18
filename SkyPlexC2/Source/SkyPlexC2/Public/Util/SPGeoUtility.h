// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SPGeoUtility.generated.h"

namespace SPGeoMath {
	constexpr double TWO_PI_ = 6.28318530717958647692;
	constexpr double EPSILON12 = 1e-12;
	constexpr int METERS_PER_LAT = 111139;

	/* This is a rough approximation. True values must be computed based on latitude. */
	constexpr int METERS_PER_LON = 111320;
}

//
// Google Geo Schemas
//

USTRUCT()
struct FGoogleGeoLatLon {
	GENERATED_BODY()

	UPROPERTY()
	double lat;

	UPROPERTY()
	double lon;
};

USTRUCT()
struct FGoogleGeoViewport {
	GENERATED_BODY()

	UPROPERTY()
	FGoogleGeoLatLon northeast;

	UPROPERTY()
	FGoogleGeoLatLon southwest;
};

USTRUCT()
struct FGoogleGeoGeometry {
	GENERATED_BODY()

	UPROPERTY()
	FGoogleGeoLatLon location;

	UPROPERTY()
	FString location_type;

	UPROPERTY()
	FGoogleGeoViewport viewport;
};

USTRUCT()
struct FGoogleGeoAddressComponent {
	GENERATED_BODY()

	UPROPERTY()
	FString long_name;

	UPROPERTY()
	FString short_name;

	UPROPERTY()
	TArray<FString> types;
};

USTRUCT()
struct FGoogleGeoPlusCode
{
	GENERATED_BODY()

	UPROPERTY()
	FString compound_code;

	UPROPERTY()
	FString global_code;
};

USTRUCT()
struct FGoogleGeoResult
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FGoogleGeoAddressComponent> address_components;

	UPROPERTY()
	FString formatted_address;

	UPROPERTY()
	FGoogleGeoGeometry geometry;

	UPROPERTY()
	FString place_id;

	UPROPERTY()
	FGoogleGeoPlusCode plus_code;

	UPROPERTY()
	TArray<FString> types;
};

USTRUCT()
struct FGoogleGeoResponse
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FGoogleGeoResult> results;

	UPROPERTY()
	FString status;
};

//
// Google autocomplete schemas
//

USTRUCT(BlueprintType)
struct FPlacePredictionMatch {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int startOffser;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int endOffset;
};

USTRUCT(BlueprintType)
struct FStructuredFormatSecondaryText {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString text;
};

USTRUCT(BlueprintType)
struct FStructuredFormatMainText {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString text;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FPlacePredictionMatch> matches;
};

USTRUCT(BlueprintType)
struct FStructuredFormat {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FStructuredFormatMainText mainText;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FStructuredFormatSecondaryText secondaryText;
};

USTRUCT(BlueprintType)
struct FPlacePredictionText {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString text;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FPlacePredictionMatch> matches;
};

USTRUCT(BlueprintType)
struct FPlacePrediction {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString place;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString placeId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FPlacePredictionText text;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FStructuredFormat structuredFormat;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> types;
};

USTRUCT(BlueprintType)
struct FSuggestion {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FPlacePrediction placePrediction;
};

//
// Google place details schemas
//

USTRUCT(BlueprintType)
struct FGooglePlacesLocation {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double latitude;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double longitude;
};

//
// Result structs
//

USTRUCT(BlueprintType)
struct FGoogleAPIAddressPredictionResult {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FSuggestion> suggestions;
};

USTRUCT(BlueprintType)
struct FGooglePlacesResult {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGooglePlacesLocation location;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString formattedAddress;
};

USTRUCT()
struct FCoordinatesToAddressResult {
	GENERATED_BODY()

	UPROPERTY()
	bool didFail;

	UPROPERTY()
	FString address;
};

/**
 * Geographic utility functions not yet provided by Cesium for Unreal
 * Also includes utility functions to query google geo apis
 *		- the google utility functions should probably be moved to the typescript backend in the future to leverage caching and localization
 *		  but this works for now
 */
UCLASS()
class SKYPLEXC2_API USPGeoUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Call's VincentyInverseFormula with a WGS84 ellipsoid. Distance is in meters. */
	UFUNCTION(BlueprintCallable)
	static double GeodesicDistance(FVector LonLatHeightA, FVector LonLatHeightB);

	// Lon and lats must be in radians, not degrees
	static double VincentyInverseFormula(double Major, double Minor, double Lon1, double Lon2, double Lat1, double Lat2);

	static TFuture<FCoordinatesToAddressResult> CoordinatesToAddress(float latitude, float longitude);

	/**
	* Pass in a PlaceID from the AddressPredictions method to get more details about the location
	* and terminate the address prediction session
	*/
	static TFuture<FGooglePlacesResult> PlaceDetails(FString PlaceID);

	static TFuture<FGoogleAPIAddressPredictionResult> AddressPredictions(FString AddressSearchString);

	UFUNCTION(BlueprintCallable)
	static void ClearAddressPredictionSession();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Geographic")
	static const double MetersFromLatitude(double Latitude, double Meters);

	/* Setting latitude to -999 will return a faster but less-accurate value */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Geographic")
	static const double MetersFromLongitude(double Longitude, double Meters, double Latitude = -999.0f);

	static FVector LatLonToLocal(
		class ACesiumGeoreference* Georeference,
		const FVector& OriginLonLatHeight,
		const FVector& PointLonLatHeight
	);

	static TArray<FVector> LatLonToLocal(
		ACesiumGeoreference* Georeference,
		const FVector& OriginLonLatHeight,
		const TArray<FVector>& PointLonLatHeights
	);

	static FVector LocalToLatLon(
		ACesiumGeoreference* Georeference,
		const FVector& OriginLonLatHeight,
		const FVector& PointLocal
	);

	static TArray<FVector> LocalToLatLon(
		ACesiumGeoreference* Georeference,
		const FVector& OriginLonLatHeight,
		const TArray<FVector>& PointLocals
	);

	static TArray<FVector> RotatePoints(
		const TArray<FVector>& LocalPoints,
		double AngleDeg
	);

	static TArray<FVector2D> GetPolygonLineIntersections(
		const TArray<FVector>& PolyPointsLocal,
		double LineY
	);

private:
	static double ComputeDeltaLambda(
		const double& Flattening,
		const double& SinAlpha,
		const double& CosSquaredAlpha,
		const double& Sigma,
		const double& SinSigma,
		const double& CosSigma,
		const double& CosTwiceSigmaMidpoint
	);

	static void StartAddressPredictionSession();

	static FString AddressPredictionSessionID;
};
