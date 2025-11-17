// Copyright (c) 2025 Synetos Aerospace

using System.IO;
using UnrealBuildTool;

public class SkyPlexC2 : ModuleRules
{
	public SkyPlexC2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;

        string ThirdPartyDirectory = Path.Combine(ModuleDirectory, "ThirdParty");

		string SQLitePath = Path.Combine(ThirdPartyDirectory, "SQLite");
		string SQLiteDLLPath = Path.Combine(SQLitePath, "Win64", "sqlite3.dll");
		string SQLiteIncludePath = Path.Combine(SQLitePath, "Include");

        if (!File.Exists(SQLiteDLLPath))
        {
            throw new FileNotFoundException("sqlite3.dll not found at " + SQLiteDLLPath);
        }

        PrivateIncludePaths.AddRange(new string[] {
            "SkyPlexC2/Private",
        });

        PublicIncludePaths.AddRange(new string[] {
            "SkyPlexC2/Public",
            SQLiteIncludePath,
        });

        PublicDependencyModuleNames.AddRange(new string[] {
			"CesiumRuntime",
			"Core",
			"CoreUObject",
			"HTTP",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"Json",
			"JsonUtilities",
			"WebSockets",
			"ProceduralMeshComponent"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		RuntimeDependencies.Add("$(BinaryOutputDir)/sqlite3.dll", SQLiteDLLPath);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string SQLiteLibPath = Path.Combine(SQLitePath, "Win64", "sqlite3.lib");

			PublicAdditionalLibraries.AddRange(new string[]
			{
				SQLiteLibPath,
			});
		}

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
