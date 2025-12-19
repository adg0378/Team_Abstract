#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GoogleAPIs.generated.h"

// Forward declarations
class USPLogger;

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
 * Google APIs wrapper
 */
UCLASS()
class SKYPLEXC2_API UGoogleAPIs : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TFuture<FCoordinatesToAddressResult> CoordinatesToAddress(USPLogger* log, float latitude, float longitude);

	/** 
	* Pass in a PlaceID from the AddressPredictions method to get more details about the location
	* and terminate the address prediction session
	*/
	static TFuture<FGooglePlacesResult> PlaceDetails(FString PlaceID);

	static TFuture<FGoogleAPIAddressPredictionResult> AddressPredictions(FString AddressSearchString);

	UFUNCTION(BlueprintCallable)
	static void ClearAddressPredictionSession();

private:

	static void StartAddressPredictionSession();

	static FString AddressPredictionSessionID;
};
