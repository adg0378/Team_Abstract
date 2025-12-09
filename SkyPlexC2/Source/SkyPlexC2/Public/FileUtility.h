// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"

/**
 * File helpers
 */
class SKYPLEXC2_API FileUtility
{
public:
	static bool WriteStringToFile(const FString& filepath, const FString& contents);
};
