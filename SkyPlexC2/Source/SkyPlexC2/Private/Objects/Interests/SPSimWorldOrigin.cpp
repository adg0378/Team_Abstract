// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Interests/SPSimWorldOrigin.h"

void ASPSimWorldOrigin::GetInteractionBoxTitle_Implementation(FText& OutTitle) {
    OutTitle = FText::FromString(TEXT("Simulation Origin"));
}

void ASPSimWorldOrigin::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
    FVector LonLatHeight;
    GetLongitudeLatitudeHeight(LonLatHeight);

    OutKeyVals.Add(TEXT("Position"), FInteractionBoxValue{
        .displayType = EKeyValDisplayType::UneditableText,
        .value = FText::FromString(FString::Printf(TEXT("%lf, %lf"), LonLatHeight.Y, LonLatHeight.X))
        });

    OutKeyVals.Add(TEXT("Drone Simulation Radius (nm)"), FInteractionBoxValue{
        .displayType = EKeyValDisplayType::UneditableText,
        .value = FText::FromString(FString::Printf(TEXT("%f"), PermittedRadiusNM))
        });
}