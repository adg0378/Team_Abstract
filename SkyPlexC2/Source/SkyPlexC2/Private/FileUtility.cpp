// Copyright (c) 2025 Synetos Aerospace


#include "FileUtility.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

bool FileUtility::WriteStringToFile(const FString& filepath, const FString& contents) {
	FString directory = FPaths::GetPath(filepath);
	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*directory))
	{
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*directory);
	}

	return FFileHelper::SaveStringToFile(contents, *filepath);
}
