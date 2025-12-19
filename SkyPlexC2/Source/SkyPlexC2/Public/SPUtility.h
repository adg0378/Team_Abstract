// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "SPUtility.generated.h"

// forward declarations
class ASPGameState;
class USPLogger;
class ASPPlayerController;
class UCesiumGlobeAnchorComponent;

UENUM(BlueprintType)
enum class EState : uint8
{
	Unassigned UMETA(DisplayName = "Unassigned"),
	Paused UMETA(DisplayName = "Paused"),
	Error UMETA(DisplayName = "Error"),
	Running UMETA(DisplayName = "Running"),
	Selected UMETA(DisplayName = "Selected"),
};

/**
 * Common utility functions for SkyPlexC2
 */
UCLASS()
class SKYPLEXC2_API USPUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "CPPUtil", meta = (WorldContext = "worldContextObject"))
	static UWorld* GetWorldContext(UObject* worldContextObject);

	UFUNCTION(BlueprintCallable, Category = "CPPUtil", meta = (WorldContext = "worldContextObject"))
	static ASPGameState* GetSPGameState(const UObject* worldContextObject);

	UFUNCTION(BlueprintCallable, Category = "CPPUtil", meta = (WorldContext = "worldContextObject"))
	static ASPPlayerController* GetSPPlayerController(UObject* worldContextObject);

	UFUNCTION(BlueprintCallable, Category = "CPPUtil", meta = (WorldContext = "worldContextObject"))
	static APlayerController* GetRegularPlayerController(UObject* worldContextObject);

	UFUNCTION(BlueprintCallable, Category = "CPPUtil", meta = (WorldContext = "worldContextObject"))
	static USPLogger* GetLogger(UObject* worldContextObject);

	UFUNCTION(BlueprintCallable, Category = "CPPUtil", meta = (WorldContext = "worldContextObject"))
	static UMaterialInstance* StateToMaterial(UObject* worldContextObject, EState state);

	UFUNCTION(BlueprintCallable, Category = "CPPUtil")
	static FText StateToDisplayName(EState state);

	UFUNCTION(BlueprintCallable, Category = "CPPUtil", meta = (WorldContext = "worldContextObject"))
	static void MoveActorToMousePosition(UObject* worldContextObject, AActor* actor, FVector mouseOffset, bool moveAllSelected = false);

	UFUNCTION(BlueprintCallable, Category = "CPPUtil", meta = (WorldContext = "worldContextObject"))
	static void SetActorLocationAboveCesiumTerrain(UObject* worldContextObject, AActor* actor, FVector offsetFromCurrentLocation = FVector(0));

	// might assist with other positioning issues but doesn't work
	/*UFUNCTION(BlueprintCallable, Category = "CPPUtil", meta = (WorldContext = "worldContextObject"))
	static void SetActorLocationAboveCesiumTerrainWithGlobeAnchor(UObject* worldContextObject, AActor* actor, UCesiumGlobeAnchorComponent* globeAnchor);*/

	static ECollisionChannel GetLandscapeTraceChannel();

	static ETraceTypeQuery GetLandscapeTraceType();

private:
	static TMap<EState, UMaterialInstance*> stateMaterials;

	static TEnumAsByte<ECollisionChannel> landscapeTraceChannel;

	static TEnumAsByte<ETraceTypeQuery> landscapeTraceType;

	UFUNCTION(meta = (WorldContext = "worldContextObject"))
	static void BuildStateMaterialsMap(UObject* worldContextObject);

	static void BuildTraceChannels();
};
