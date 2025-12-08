// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/SPInteractable.h"
#include "Util/SPObstacleUtility.h"
#include "SPADSB.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API ASPADSB : public ASPInteractable
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable)
    void SetTrailLength(int Length);

    UFUNCTION(BlueprintCallable)
    void GetData(FADSBAircraftStruct& OutData) const;

    UFUNCTION(BlueprintCallable)
    void SetData(const FADSBAircraftStruct& InData);

    void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) override;
    void GetInteractionBoxTitle_Implementation(FText& OutTitle) override;

protected:
    virtual void BeginPlay() override;

private:
    FADSBAircraftStruct Data;

    UPROPERTY();
    TObjectPtr<class ASPFlightTrail> Trail;
};
