// ====================
// ADSBObject.h
// ====================

// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/Interactable.h"
#include "State/Obstacles/ObstaclesUtil.h"
#include "ADSBObject.generated.h"

// forward declarations
class ASPFlightTrail;

UCLASS(BlueprintType, Blueprintable)
class SKYPLEXC2_API AADSBObject : public AInteractable
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
    virtual void ToggleCull_Implementation(bool IsCulled, bool FromFlyTo) override;

protected:
    virtual void BeginPlay() override;

private:
    FADSBAircraftStruct Data;

    UPROPERTY();
    TObjectPtr<ASPFlightTrail> Trail;
};