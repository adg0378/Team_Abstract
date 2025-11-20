// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Obstacles/SPADSB.h"
#include "Objects/Misc/SPFlightTrail.h"
#include "CesiumGlobeAnchorComponent.h"
#include "Core/State/SPGameState.h"
#include "Core/State/SPSelectionManager.h"
#include "CesiumGeoreference.h"

void ASPADSB::BeginPlay() {
    Super::BeginPlay();

    Trail = GetWorld()->SpawnActor<ASPFlightTrail>();

    if (Trail) {
        Trail->SetTrailLimit(10);
        Trail->SetTrailColor(FLinearColor::Red);
    }
}

void ASPADSB::SetTrailLength(int Length) {
    Trail->SetTrailLimit(Length);
}

void ASPADSB::SetData(const FADSBAircraftStruct& InData) {
    Data = InData;

    ACesiumGeoreference* Georeference = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
    FVector Location = this->GetActorLocation();
    FRotator Rotation = this->GetActorRotation();

    float Pitch = 0.0;
    if (Data.geom_rate != 0 && Data.gs != 0) {
        Pitch = Data.geom_rate / (Data.gs * 101.269);
    }

    FVector SpawnPosition(Data.lon, Data.lat, Data.alt_geom * 0.3048);
    Trail->AddTrailPointCoord(SpawnPosition);

    // initialize at -90 because we are setting EastSouthUp. We must subtract -90 to set based on North
    float Yaw = -90;
    Yaw += Data.track != 0 ? Data.track : Data.true_heading;

    this->GlobeAnchorComponent->SetEastSouthUpRotation(FQuat::MakeFromEuler(FVector(Data.roll, Pitch, Yaw)));

    if (_IsSelected) {
        ASPGameState* gameState = ASPGameState::GetSPGameState(Cast<UObject>(this));
        gameState->SelectionManager->RefreshSelected();
    }
}

void ASPADSB::GetData(FADSBAircraftStruct& OutData) const {
    OutData = Data;
}

void ASPADSB::GetInteractionBoxTitle_Implementation(FText& OutTitle) {
    OutTitle = FText::FromString(FString::Printf(TEXT("%s"), *Data.flight));
}

void ASPADSB::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
    OutKeyVals.Add(TEXT("Flight"), FInteractionBoxValue{
        .displayType = EKeyValDisplayType::UneditableText,
        .value = FText::FromString(Data.flight)
        });

    OutKeyVals.Add(TEXT("ICAO ID"), FInteractionBoxValue{
        .displayType = EKeyValDisplayType::UneditableText,
        .value = FText::FromString(Data.hex)
        });

    OutKeyVals.Add(TEXT("Coordinates"), FInteractionBoxValue{
        .displayType = EKeyValDisplayType::UneditableText,
        .value = FText::FromString(FString::Printf(TEXT("%.6f, %.6f"), Data.lat, Data.lon))
        });

    OutKeyVals.Add(TEXT("Barometric Altitude"), FInteractionBoxValue{
        .displayType = EKeyValDisplayType::UneditableText,
        .value = FText::FromString(Data.alt_baro.IsNumeric() ? FString::Printf(TEXT("%s ft"), *Data.alt_baro) : *Data.alt_baro)
        });

    if (Data.alt_geom != 0) {
        OutKeyVals.Add(TEXT("Geometric Altitude"), FInteractionBoxValue{
            .displayType = EKeyValDisplayType::UneditableText,
            .value = FText::FromString(FString::Printf(TEXT("%i"), Data.alt_geom))
            });
    }

    if (Data.geom_rate != 0) {
        OutKeyVals.Add(TEXT("Geom Rate"), FInteractionBoxValue{
            .displayType = EKeyValDisplayType::UneditableText,
            .value = FText::FromString(FString::Printf(TEXT("%i"), Data.geom_rate))
            });
    }

    if (Data.true_heading != 0.0) {
        OutKeyVals.Add(TEXT("True Heading"), FInteractionBoxValue{
            .displayType = EKeyValDisplayType::UneditableText,
            .value = FText::FromString(FString::Printf(TEXT("%.2f"), Data.true_heading))
            });
    }

    if (Data.track != 0.0) {
        OutKeyVals.Add(TEXT("Track"), FInteractionBoxValue{
            .displayType = EKeyValDisplayType::UneditableText,
            .value = FText::FromString(FString::Printf(TEXT("%.2f"), Data.track))
            });
    }

    OutKeyVals.Add(TEXT("Ground Speed"), FInteractionBoxValue{
        .displayType = EKeyValDisplayType::UneditableText,
        .value = FText::FromString(FString::Printf(TEXT("%.1f kts"), Data.gs))
        });

    OutKeyVals.Add(TEXT("Category"), FInteractionBoxValue{
        .displayType = EKeyValDisplayType::UneditableText,
        .value = FText::FromString(Data.category)
        });

    OutKeyVals.Add(TEXT("Squawk"), FInteractionBoxValue{
        .displayType = EKeyValDisplayType::UneditableText,
        .value = FText::FromString(Data.squawk)
        });
}