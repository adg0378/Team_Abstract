// Copyright (c) 2025 Synetos Aerospace


#include "Util/FileUtility.h"

bool UFileUtility::WriteStringToFile(const FString& FilePath, const FString& Contents) {
	FString directory = FPaths::GetPath(FilePath);
	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*directory))
	{
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*directory);
	}

	return FFileHelper::SaveStringToFile(Contents, *FilePath);
}