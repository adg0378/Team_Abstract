// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Objects/InteractionBoxProvider.h"
#include "Objects/PlaceableProvider.h"
#include "Polygon.generated.h"

// Forward declarations
class APlaceablePoint;

USTRUCT(BlueprintType)
struct FPolygonListNode {
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<APlaceablePoint> Point;

	TSharedPtr<FPolygonListNode> Next = nullptr;
	TWeakPtr<FPolygonListNode> Prev = nullptr;

	FPolygonListNode() = default;
	explicit FPolygonListNode(APlaceablePoint* InPoint) : Point(InPoint), Next(nullptr), Prev(nullptr) {}
};

UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API APolygon : public AActor, public IInteractionBoxProvider, public IPlaceableProvider
{
	GENERATED_BODY()
	
public:
	virtual void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals);
	virtual void GetInteractionBoxTitle_Implementation(FText& OutTitle);
	virtual void SetLinkedProvider_Implementation(const TScriptInterface<UInteractionBoxProvider>& InProvider);
	virtual void GetLinkedProvider_Implementation(TScriptInterface<UInteractionBoxProvider>& OutProvider) const;
	virtual void DestroySelf_Implementation();
	virtual void ProvideOnPlaced_Implementation();
	virtual void OnInteractionBoxTitleChanged_Implementation(const FText &NewTitle);
	virtual void ToggleCull_Implementation(bool IsCulled, bool FromFlyTo);

	UFUNCTION(BlueprintCallable)
	// Clears all points and then destroys this polygon
	void DestroyPolygon();

	UFUNCTION(BlueprintCallable)
	// Clears all points, but this polygon persists
	void ClearAllPoints();

	UFUNCTION(BlueprintCallable)
	void AddPoint(APlaceablePoint* InPoint);

	UFUNCTION(BlueprintCallable)
	void CanBeClosed(bool& OutCanBeClosed) const;

	UFUNCTION(BlueprintCallable)
	FVector GetCenterpointUE() const;
	FVector GetCenterpoint() const;

	UPROPERTY(BlueprintReadWrite)
	bool IsClosed = false;

	UFUNCTION(BlueprintCallable)
	void GetHead(APlaceablePoint*& OutPoint) const;

	/** Creates initial highlight polygon or updates it if it already exists */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Highlight();
	void Highlight_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UnHighlight();
	void UnHighlight_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void HighlightOnly(bool IsHighlighted);
	void HighlightOnly_Implementation(bool IsHighlighted);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OutlineOnly(bool IsOutlined);
	void OutlineOnly_Implementation(bool IsOutlined);

	UFUNCTION(BlueprintCallable)
	void Close();

	UFUNCTION(BlueprintCallable)
	void GetPointLocations(TArray<FVector>& OutLocations);

	UFUNCTION(BlueprintCallable)
	void GetActorLocations(TArray<FVector>& OutActorLocations);

	void ForEachPoint(const TFunctionRef<void(APlaceablePoint*)>& Callback) const;

	UFUNCTION(BlueprintCallable)
	bool GetInclusive() const;

	UFUNCTION(BlueprintCallable)
	void SetInclusive(bool IsInclusive);

private:
	UFUNCTION()
	void CloseFromClickMe();

	int Length = 0;

	// controls whether the inside of the polygon is highlighted or not
	bool Inclusive = true;

	TSharedPtr<FPolygonListNode> Head = nullptr;

	UPROPERTY()
	TScriptInterface<UInteractionBoxProvider> LinkedProvider;
};
