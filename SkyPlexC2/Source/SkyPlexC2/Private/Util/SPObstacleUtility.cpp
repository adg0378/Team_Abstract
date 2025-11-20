// Copyright (c) 2025 Synetos Aerospace


#include "Util/SPObstacleUtility.h"
#include "Util/SPHTTPUtility.h"
#include "Core/SPLogger.h"
#include "SPEnvConstants.h"

TFuture<TArray<FFAAObjectDataStruct>> USPObstacleUtility::GetFAADOFObjects(USPLogger* Log, FString LogOrigin, float Lon, float Lat, int32 RadiusNM) {
	FString RequestUrl = FString::Printf(TEXT("%sobstacles/%f/%f/%i"), *USPEnvConstants::OBSTACLES_URL, Lon, Lat, RadiusNM);

	TSharedRef<TPromise<TArray<FFAAObjectDataStruct>>> Promise = MakeShared<TPromise<TArray<FFAAObjectDataStruct>>>();

	USPHTTPUtility Client;
	Client.SetUrl(RequestUrl)
		.RequestJSON<FFAAObjectAPIResult>(HTTPRequestType::GET)
		.Then([Log, LogOrigin, Promise](TFuture<FHTTPJSONResult<FFAAObjectAPIResult>> ResFut) {
		const FHTTPJSONResult<FFAAObjectAPIResult>& Result = ResFut.Get();
		TArray<FFAAObjectDataStruct> Res;
		if (Result.didFail) {
			Log->Error(Result.errMsg, LogOrigin);
		}
		else {
			Res = Result.response.obstacles;
		}
		Promise->SetValue(Res);
			});

	return Promise->GetFuture();
}

TFuture<FADSBObjectDataResult> USPObstacleUtility::GetADSBObjects(USPLogger* Log, FString LogOrigin, float Lon, float Lat, int32 RadiusNM) {
	FString RequestUrl = FString::Printf(TEXT("%spoint/%f/%f/%i"), *USPEnvConstants::ADSB_URL, Lat, Lon, RadiusNM);

	TSharedRef<TPromise<FADSBObjectDataResult>> Promise = MakeShared<TPromise<FADSBObjectDataResult>>();

	USPHTTPUtility Client;
	Client.SetUrl(RequestUrl)
		.RequestJSON<FADSBObjectDataResult>(HTTPRequestType::GET)
		.Then([Log, LogOrigin, Promise](TFuture<FHTTPJSONResult<FADSBObjectDataResult>> ResFut) {
		const FHTTPJSONResult<FADSBObjectDataResult>& Result = ResFut.Get();
		FADSBObjectDataResult Res;
		if (Result.didFail) {
			Log->Error(Result.errMsg, LogOrigin);
		}
		else {
			Res = Result.response;
		}
		Promise->SetValue(Res);
			});

	return Promise->GetFuture();

}
