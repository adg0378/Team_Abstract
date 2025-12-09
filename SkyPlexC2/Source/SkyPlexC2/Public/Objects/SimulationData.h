//
//  SimulationData.h
//  SkyPlexC2 (Mac)
//
//  Created by Aidan Quinn on 7/28/25.
//  Copyright © 2025 Epic Games, Inc. All rights reserved.
//

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SimulationData.generated.h"

USTRUCT(BlueprintType)
struct FSimulationData : public FTableRowBase //inherit from FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation Data")
    FString SimulationTitle;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Simulation Data")
    int32 DroneCount;


};
