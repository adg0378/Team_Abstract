#include "API/GoogleAPIs.h"
#include "SPLogger.h"
#include "AsynJSONUtility.h"
#include <filesystem>
#include "API/HTTPUtility.h"

// This is really messy but we have more things to worry about than this

const FString API_KEY = TEXT("AIzaSyBuz6ptFEFxkvFccXSHEvkvyG7_3F6dVaY");

FString UGoogleAPIs::AddressPredictionSessionID;

void UGoogleAPIs::StartAddressPredictionSession() {
    FGuid NewUUID = FGuid::NewGuid();
    AddressPredictionSessionID = NewUUID.ToString();
}

void UGoogleAPIs::ClearAddressPredictionSession() {
    AddressPredictionSessionID = FString();
}

TFuture<FGoogleAPIAddressPredictionResult> UGoogleAPIs::AddressPredictions(FString AddressSearchString) {
    if (AddressPredictionSessionID.IsEmpty()) {
        StartAddressPredictionSession();
    }

    FString RequestBody = FString::Printf(TEXT("{\"input\": \"%s\",\"sessionToken\": \"%s\"}"), *AddressSearchString, *AddressPredictionSessionID);
    TSharedRef<TPromise<FGoogleAPIAddressPredictionResult>> Promise = MakeShared<TPromise<FGoogleAPIAddressPredictionResult>>();

    HTTPUtility Client;
    Client.SetUrl(TEXT("https://places.googleapis.com/v1/places:autocomplete"))
        .SetPostFields(RequestBody)
        .AppendHeader(TEXT("Content-Type"), TEXT("application/json"))
        .AppendHeader(TEXT("X-Goog-Api-Key"), *API_KEY)
        .RequestJSON<FGoogleAPIAddressPredictionResult>(HTTPRequestType::POST)
        .Then([Promise](TFuture<FHTTPJSONResult<FGoogleAPIAddressPredictionResult>> ResFut) {
            const FHTTPJSONResult<FGoogleAPIAddressPredictionResult>& Result = ResFut.Get();
            if (Result.didFail) {
                UE_LOG(LogTemp, Error, TEXT("Failed to get address predictions: %s"), *Result.errMsg)
                FGoogleAPIAddressPredictionResult Res;
                Promise->SetValue(Res);
            }
            else {
                Promise->SetValue(Result.response);
            }
        });

    return Promise->GetFuture();
}

TFuture<FGooglePlacesResult> UGoogleAPIs::PlaceDetails(FString PlaceID) {
    FString BaseUrl = TEXT("https://places.googleapis.com/v1/places/");
    FString RequestUrl = AddressPredictionSessionID.IsEmpty()
        ? FString::Printf(TEXT("%s%s"), *BaseUrl, *PlaceID)
        : FString::Printf(TEXT("%s%s?sessionToken=%s"), *BaseUrl, *PlaceID, *AddressPredictionSessionID);

    TSharedRef<TPromise<FGooglePlacesResult>> Promise = MakeShared<TPromise<FGooglePlacesResult>>();

    // Make request
    HTTPUtility Client;
    Client.SetUrl(RequestUrl)
        .AppendHeader(TEXT("X-Goog-Api-Key"), *API_KEY)
        .AppendHeader(TEXT("Content-Type"), TEXT("application/json"))
        .AppendHeader(TEXT("X-Goog-FieldMask"), TEXT("id,location,formattedAddress"))
        .RequestJSON<FGooglePlacesResult>(HTTPRequestType::GET)
        .Then([Promise](TFuture<FHTTPJSONResult<FGooglePlacesResult>> ResFut) {
            const FHTTPJSONResult<FGooglePlacesResult>& Result = ResFut.Get();
            if (Result.didFail) {
                UE_LOG(LogTemp, Error, TEXT("Failed to get coordinates from PlaceID: %s"), *Result.errMsg);
                FGooglePlacesResult Res;
                Res.location.latitude, Res.location.longitude = -999.0;
                Promise->SetValue(Res);
            }
            else {
                Promise->SetValue(Result.response);
            }
        });

    ClearAddressPredictionSession();
    return Promise->GetFuture();
}

TFuture<FCoordinatesToAddressResult> UGoogleAPIs::CoordinatesToAddress(USPLogger* log, float latitude, float longitude) {
    FString logOrigin = TEXT("CoordinatesToAddress");

    // Restrict formatted addresses to City, State, Country
    FString requestUrl = FString::Printf(
        TEXT("https://maps.googleapis.com/maps/api/geocode/json?latlng=%f,%f&key=%s&result_type=political|sublocality|sublocality_level_1"),
        latitude,
        longitude,
        *API_KEY
    );

    TSharedRef<TPromise<FCoordinatesToAddressResult>> promise = MakeShared<TPromise<FCoordinatesToAddressResult>>();

    // Make request
    HTTPUtility client;
    client.SetUrl(requestUrl)
        .RequestJSON<FGoogleGeoResponse>(HTTPRequestType::GET)
        .Then([log, logOrigin, promise](TFuture<FHTTPJSONResult<FGoogleGeoResponse>> resFut) {
            const FHTTPJSONResult<FGoogleGeoResponse>& result = resFut.Get();
            FCoordinatesToAddressResult res;
            if (result.didFail) {
                log->Error(result.errMsg, logOrigin);
                res.didFail = true;
            }
            else if (result.response.status != TEXT("OK")) {
                log->Error(TEXT("Response not OK"), logOrigin);
                res.didFail = true;
            }
            else if (result.response.results.Num() == 0) {
                res.didFail = false;
                res.address = TEXT("Unnamed location");
            }
            else {
                res.didFail = false;
                res.address = result.response.results[0].formatted_address;
            }
            promise->SetValue(res);
        });

    return promise->GetFuture();
}