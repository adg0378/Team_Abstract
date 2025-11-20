// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Obstacles/SPFAADOF.h"
#include "Objects/Geo/SPPlaceablePoint.h"

void ASPFAADOF::SetData(FFAAObjectDataStruct InData) {
	Data = InData;
	HeightAboveGroundLevelMeters = InData.agl * 0.3048;
	OnSetData();
}

void ASPFAADOF::GetData(FFAAObjectDataStruct& OutData) {
	OutData = Data;
}

void ASPFAADOF::OnSetData_Implementation() {}

void ASPFAADOF::GetInteractionBoxTitle_Implementation(FText& OutTitle) {
	OutTitle = FText::FromString(Data.oas);
}

void ASPFAADOF::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
	OutKeyVals.Add(TEXT("Verification"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(Data.verified ? TEXT("Verified") : TEXT("Unverified"))
		});

	OutKeyVals.Add(TEXT("Address"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(FString::Printf(TEXT("%s, %s, %s"), *Data.city, *Data.state, *Data.country))
		});

	OutKeyVals.Add(TEXT("Coordinates"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(FString::Printf(TEXT("%.6f, %.6f"), Data.latitude, Data.longitude))
		});

	OutKeyVals.Add(TEXT("Obstacle type"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(Data.type)
		});

	OutKeyVals.Add(TEXT("Quantity"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::AsNumber(Data.quantity)
		});

	OutKeyVals.Add(TEXT("Height"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(FString::Printf(TEXT("%.2f ft"), Data.agl))
		});

	OutKeyVals.Add(TEXT("Horizontal Accuracy"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(Data.horizontalAccuracy)
		});

	OutKeyVals.Add(TEXT("Vertical Accuracy"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(Data.verticalAccuracy)
		});

	OutKeyVals.Add(TEXT("Marking"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(Data.marking)
		});

	OutKeyVals.Add(TEXT("FAA study"), FInteractionBoxValue{
		.displayType = EKeyValDisplayType::UneditableText,
		.value = FText::FromString(Data.faaStudy)
		});
}