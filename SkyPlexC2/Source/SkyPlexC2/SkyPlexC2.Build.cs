using System.IO;
using UnrealBuildTool;

public class SkyPlexC2 : ModuleRules
{
	public SkyPlexC2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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
            SQLitePath,
        });

        PublicDependencyModuleNames.AddRange(new string[] {
            "CesiumRuntime",
            "Core",
            "CoreUObject",
            "HTTP",
            "Engine",
            "InputCore",
            "Json",
            "JsonUtilities",
            "WebSockets",
            "ProceduralMeshComponent"
        });

        PrivateIncludePathModuleNames.AddRange(new string[]
        {
            "AutomationTest",
        });

        // Enables Spec Tests and other automated unit tests
        if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "ApplicationCore",
                "AutomationTest",
                "CesiumEditor",
                "Projects",
                "Engine",
                "CoreUObject"
            });
        }

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string SQLiteLibPath = Path.Combine(SQLitePath, "Win64", "sqlite3.lib");

            PublicAdditionalLibraries.AddRange(new string[]
            {
                SQLiteLibPath,
            });

            RuntimeDependencies.Add("$(BinaryOutputDir)/sqlite3.dll", SQLiteDLLPath);
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicSystemLibraries.Add("sqlite3");
        }

        //if (Target.Platform == UnrealTargetPlatform.Win64) {
        //    string SQLiteLibPath = Path.Combine(SQLitePath, "sqlite3.lib");
        //    string zlibLibPath = Path.Combine(zlibPath, "zlib.lib");
        //    string MAVLibraryPath = Path.Combine(MAVSDKPath, "mavsdk.lib");
        //    string minizipLibPath = Path.Combine(minizipPath, "minizip.lib");

        //    PublicAdditionalLibraries.AddRange(new string[] {
        //        SQLiteLibPath,
        //        zlibLibPath,
        //        MAVLibraryPath,
        //        minizipLibPath,
        //    });
        //}
        
        //else if (Target.Platform == UnrealTargetPlatform.Mac) {
        //    string SQLiteLibPath = Path.Combine(SQLitePath, "libsqlite3.a");
        //    string minizipLibPath = Path.Combine(minizipPath, "libminizip.a");
        //    PublicAdditionalLibraries.AddRange(new string[] {
        //        minizipLibPath,
        //        SQLiteLibPath,
        //    });
            
        //    // mavsdk stuff
        //    PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "../../Binaries/Mac/libmavsdk.3.2.0.dylib"));
            
        //    RuntimeDependencies.Add(Path.Combine(ModuleDirectory, "../../Binaries/Mac/libmavsdk.3.2.0.dylib"));
        //}

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
