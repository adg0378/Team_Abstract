// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintUtility.generated.h"

/**
 * 
 */
UCLASS()
class SKYPLEXC2_API UBlueprintUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BlueprintUtility")
	static void CoordToStringSix(double inFloat, FString& outString);

	UFUNCTION(BlueprintCallable, Category = "BlueprintUtility")
	static void CoordToStringEight(double inFloat, FString& outString);
};
