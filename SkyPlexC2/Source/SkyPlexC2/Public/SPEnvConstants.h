// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SPEnvConstants.generated.h"

/**
 * 
 */
UCLASS()
class SKYPLEXC2_API USPEnvConstants : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static FString GetUserSavePath();
	static FString GetCameraLocationPath();
	static FString GetPreferencesPath();
	static FString GetFlyToLocationPath();
	static FString GetDatabasePath();
	static FString GetObstacleCachePath();

	static const FString FIREBASE_URL;
	static const FString GOOGLE_TOKEN_URL;
	static const FString OBSTACLES_URL;
	static const FString ADSB_URL;
};
