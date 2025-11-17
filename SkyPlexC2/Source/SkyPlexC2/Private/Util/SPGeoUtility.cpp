// Copyright (c) 2025 Synetos Aerospace


#include "Util/SPGeoUtility.h"
#include "Util/SPHTTPUtility.h"
#include "Core/SPLogger.h"
#include "CesiumGeospatial/Ellipsoid.h"

// This is a sin but there isn't a good way to safely store keys like this externally
// This is another reason why we should move to the typescript backend
const FString API_KEY = TEXT("AIzaSyALg3znSk9ox97JdvuTo3_xbbwsQS57_gs");

FString USPGeoUtility::AddressPredictionSessionID;

void USPGeoUtility::StartAddressPredictionSession() {
	FGuid NewUUID = FGuid::NewGuid();
	AddressPredictionSessionID = NewUUID.ToString();
}

void USPGeoUtility::ClearAddressPredictionSession() {
	AddressPredictionSessionID = FString();
}

TFuture<FGoogleAPIAddressPredictionResult> USPGeoUtility::AddressPredictions(FString AddressSearchString) {
	if (AddressPredictionSessionID.IsEmpty()) {
		StartAddressPredictionSession();
	}

	FString RequestBody = FString::Printf(TEXT("{\"input\": \"%s\",\"sessionToken\": \"%s\"}"), *AddressSearchString, *AddressPredictionSessionID);
	TSharedRef<TPromise<FGoogleAPIAddressPredictionResult>> Promise = MakeShared<TPromise<FGoogleAPIAddressPredictionResult>>();

	USPHTTPUtility Client;
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

TFuture<FGooglePlacesResult> USPGeoUtility::PlaceDetails(FString PlaceID) {
	FString BaseUrl = TEXT("https://places.googleapis.com/v1/places/");
	FString RequestUrl = AddressPredictionSessionID.IsEmpty()
		? FString::Printf(TEXT("%s%s"), *BaseUrl, *PlaceID)
		: FString::Printf(TEXT("%s%s?sessionToken=%s"), *BaseUrl, *PlaceID, *AddressPredictionSessionID);

	TSharedRef<TPromise<FGooglePlacesResult>> Promise = MakeShared<TPromise<FGooglePlacesResult>>();

	// Make request
	USPHTTPUtility Client;
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

TFuture<FCoordinatesToAddressResult> USPGeoUtility::CoordinatesToAddress(float latitude, float longitude) {

	// Restrict formatted addresses to City, State, Country
	FString requestUrl = FString::Printf(
		TEXT("https://maps.googleapis.com/maps/api/geocode/json?latlng=%f,%f&key=%s&result_type=political|sublocality|sublocality_level_1"),
		latitude,
		longitude,
		*API_KEY
	);

	TSharedRef<TPromise<FCoordinatesToAddressResult>> promise = MakeShared<TPromise<FCoordinatesToAddressResult>>();

	// Make request
	USPHTTPUtility client;
	client.SetUrl(requestUrl)
		.RequestJSON<FGoogleGeoResponse>(HTTPRequestType::GET)
		.Then([promise](TFuture<FHTTPJSONResult<FGoogleGeoResponse>> resFut) {
		const FHTTPJSONResult<FGoogleGeoResponse>& result = resFut.Get();
		FCoordinatesToAddressResult res;
		if (result.didFail) {
			UE_LOG(LogTemp, Error, TEXT("%s"), *result.errMsg);
			res.didFail = true;
		}
		else if (result.response.status != TEXT("OK")) {
			UE_LOG(LogTemp, Error, TEXT("Coordinate to address response not OK"));
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

double USPGeoUtility::GeodesicDistance(FVector LonLatHeightA, FVector LonLatHeightB) {
	const CesiumGeospatial::Ellipsoid& Ellipsoid = CesiumGeospatial::Ellipsoid::WGS84;
	return USPGeoUtility::VincentyInverseFormula(
		Ellipsoid.getMaximumRadius(),
		Ellipsoid.getMinimumRadius(),
		FMath::DegreesToRadians(LonLatHeightA.X),
		FMath::DegreesToRadians(LonLatHeightB.X),
		FMath::DegreesToRadians(LonLatHeightA.Y),
		FMath::DegreesToRadians(LonLatHeightB.Y)
	);
}

double USPGeoUtility::VincentyInverseFormula(double Major, double Minor, double Lon1, double Lon2, double Lat1, double Lat2) {
	double Flattening = (Major - Minor) / Major;
	double L = Lon1 - Lon2;

	double U1 = FMath::Atan((1 - Flattening) * FMath::Tan(Lat1));
	double U2 = FMath::Atan((1 - Flattening) * FMath::Tan(Lat2));

	double CosU1 = FMath::Cos(U1);
	double SinU1 = FMath::Sin(U1);
	double CosU2 = FMath::Cos(U2);
	double SinU2 = FMath::Sin(U2);

	double CosCos = CosU1 * CosU2;
	double CosSin = CosU1 * SinU2;
	double SinSin = SinU1 * SinU2;
	double SinCos = SinU1 * CosU2;

	double Lambda = L;
	double LambdaDot = SPGeoMath::TWO_PI_;

	double CosLambda = FMath::Cos(Lambda);
	double SinLambda = FMath::Sin(Lambda);

	double Sigma;
	double CosSigma;
	double SinSigma;
	double CosSquaredAlpha;
	double CosTwiceSigmaMidpoint;

	do {
		CosLambda = FMath::Cos(Lambda);
		SinLambda = FMath::Sin(Lambda);

		double Temp = CosSin - SinCos * CosLambda;
		SinSigma = FMath::Sqrt(CosU2 * CosU2 * SinLambda * SinLambda + Temp * Temp);
		CosSigma = SinSin + CosCos * CosLambda;

		Sigma = FMath::Atan2(SinSigma, CosSigma);

		double SinAlpha;

		if (SinSigma == 0.0) {
			SinAlpha = 0.0;
			CosSquaredAlpha = 1.0;
		}
		else {
			SinAlpha = (CosCos * SinLambda) / SinSigma;
			CosSquaredAlpha = 1.0 - SinAlpha * SinAlpha;
		}

		LambdaDot = Lambda;

		CosTwiceSigmaMidpoint = CosSigma - (2.0 * SinSin) / CosSquaredAlpha;

		if (!FMath::IsFinite(CosTwiceSigmaMidpoint)) {
			CosTwiceSigmaMidpoint = 0.0;
		}

		Lambda = L + USPGeoUtility::ComputeDeltaLambda(Flattening, SinAlpha, CosSquaredAlpha, Sigma, SinSigma, CosSigma, CosTwiceSigmaMidpoint);
	} while (FMath::Abs(Lambda - LambdaDot) > SPGeoMath::EPSILON12);

	double USquared = (CosSquaredAlpha * (Major * Major - Minor * Minor)) / (Minor * Minor);
	double A = 1.0 +
		(USquared *
			(4096.0 + USquared * (USquared * (320.0 - 175.0 * USquared) - 768.0))) /
		16384.0;
	double B =
		(USquared *
			(256.0 + USquared * (USquared * (74.0 - 47.0 * USquared) - 128.0))) /
		1024.0;
	double CosSquaredTwiceSigmaMidpoint = CosTwiceSigmaMidpoint * CosTwiceSigmaMidpoint;
	double DeltaSigma =
		B *
		SinSigma *
		(CosTwiceSigmaMidpoint +
			(B *
				(CosSigma * (2.0 * CosSquaredTwiceSigmaMidpoint - 1.0) -
					(B *
						CosTwiceSigmaMidpoint *
						(4.0 * SinSigma * SinSigma - 3.0) *
						(4.0 * CosSquaredTwiceSigmaMidpoint - 3.0)) /
					6.0)) /
			4.0);
	double Distance = Minor * A * (Sigma - DeltaSigma);
	return Distance;
}

double USPGeoUtility::ComputeDeltaLambda(
	const double& Flattening,
	const double& SinAlpha,
	const double& CosSquaredAlpha,
	const double& Sigma,
	const double& SinSigma,
	const double& CosSigma,
	const double& CosTwiceSigmaMidpoint
) {
	double C = (Flattening * CosSquaredAlpha * (4.0 + Flattening * (4.0 - 3.0 * CosSquaredAlpha))) / 16.0;
	return (1.0 - C) *
		Flattening *
		SinAlpha *
		(Sigma +
			C *
			SinSigma *
			(CosTwiceSigmaMidpoint +
				C *
				CosSigma *
				(2.0 * CosTwiceSigmaMidpoint * CosTwiceSigmaMidpoint - 1.0)));
}