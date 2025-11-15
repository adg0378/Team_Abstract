// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Core/State/SPManagerBase.h"
#include "Delegates/DelegateCombinations.h"
#include "SPPlacementManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlacementModeExitedSignature);

UENUM(BlueprintType)
enum class EControllerMode : uint8
{
	Default UMETA(DisplayName = "Default"),
	MultiSelect UMETA(DisplayName = "MultiSelect"),
	Placement UMETA(DisplayName = "Placement"),
	Drag UMETA(DisplayName = "Drag"),
};

/**
 * Manages the placement of actors on the tileset
 */
UCLASS()
class SKYPLEXC2_API USPPlacementManager : public USPManagerBase, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static void PositionActorOnTileset(UObject* WorldContextObject, AActor* Actor, FVector OffsetFromCurrentLocation = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static void PositionActorOnTilesetFromMousePos(UObject* WorldContextObject, AActor* Actor, FVector OffsetFromMouse = FVector::ZeroVector, bool MoveAllSelected = false);

	USPPlacementManager();

	UFUNCTION(BlueprintCallable)
	void OnPlacementInfoUpdated(TSubclassOf<class ASPPlaceable> InPlaceableActorType, enum EControllerMode ControllerMode);

	UFUNCTION(BlueprintCallable)
	void SetPlacementModeEnabled(bool IsEnabled);

	UFUNCTION(BlueprintCallable)
	void SpawnActor();

	UFUNCTION(BlueprintCallable)
	void StopDraggingObject();

	UFUNCTION(BlueprintCallable)
	bool IsPlacementModeEnabled() const;

	void Tick(float DeltaTime) override;
	bool IsTickable() const override;
	bool IsTickableInEditor() const override;
	bool IsTickableWhenPaused() const override;
	TStatId GetStatId() const override;

	static constexpr float TRACE_DISTANCE = 20000000.0f;

	/* If mode is not default or multi select, retains current mode */
	UFUNCTION(BlueprintCallable)
	void ToggleMultipleSelectMode();

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	EControllerMode Mode;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class ASPPlaceable> DraggedObject;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class ASPPolygon> InProgressPolygon;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<ASPPlaceable> PlaceableActor;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<ASPPlaceable> PlaceableActorType;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Placement")
	TSubclassOf<class ASPPlaceablePoint> POIPointToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Placement")
	TSubclassOf<ASPPlaceablePoint> TakeoffPointToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Placement")
	TSubclassOf<ASPPlaceablePoint> AOIPointToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Placement")
	TSubclassOf<ASPPolygon> AOIPolygonToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Placement")
	TSubclassOf<ASPPlaceablePoint> GeoFencePointToSpawn;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Placement")
	TSubclassOf<ASPPolygon> GeoFencePolygonToSpawn;

	UPROPERTY(BlueprintAssignable, Category = "Placement")
	FPlacementModeExitedSignature OnPlacementModeExited;

private:
	bool PlacementModeEnabled = false;

	void InitializeInProgressPolygon(ASPGameState* GameState);
};
