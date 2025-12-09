// Copyright (c) 2025 Synetos Aerospace


#include "BlueprintUtility.h"

void UBlueprintUtility::CoordToStringSix(double inFloat, FString& outString) {
    outString = FString::Printf(TEXT("%.6f"), inFloat);
}

void UBlueprintUtility::CoordToStringEight(double inFloat, FString& outString) {
    outString = FString::Printf(TEXT("%.8f"), inFloat);
}