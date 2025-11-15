// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FileUtility.generated.h"

/**
 * 
 */
UCLASS()
class SKYPLEXC2_API UFileUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool WriteStringToFile(const FString& FilePath, const FString& Contents);
};
