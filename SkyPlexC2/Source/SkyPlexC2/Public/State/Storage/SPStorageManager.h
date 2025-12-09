// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "State/SPManager.h"
#include "SPStorageManager.generated.h"

// Forward declarations
class UFlyToLocationManager;
class UMissionAssetsManager;
class UGeoJsonManager;

UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API USPStorageManager : public USPManager
{
	GENERATED_BODY()

public:
	USPStorageManager();
	~USPStorageManager();

	void Setup_Implementation(bool& outSuccess) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Storage")
	TObjectPtr<UFlyToLocationManager> flyToLocationManagerRef;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Storage")
	TObjectPtr<UMissionAssetsManager> missionAssetsManagerRef;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Storage")
	TObjectPtr<UGeoJsonManager> geoJsonManagerRef;

};
