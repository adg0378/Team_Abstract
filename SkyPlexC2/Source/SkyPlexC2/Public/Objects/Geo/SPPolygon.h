// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Objects/SPInteractionInterface.h"
#include "Objects/SPPlaceableInterface.h"
#include "SPPolygon.generated.h"

USTRUCT(BlueprintType)
struct FPolygonListNode {
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<class ASPPlaceablePoint> Point;

	TSharedPtr<FPolygonListNode> Next = nullptr;
	TWeakPtr<FPolygonListNode> Prev = nullptr;

	FPolygonListNode() = default;
	explicit FPolygonListNode(ASPPlaceablePoint* InPoint) : Point(InPoint), Next(nullptr), Prev(nullptr) {}
};

UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API ASPPolygon : public AActor, public ISPInteractionInterface, public ISPPlaceableInterface
{
	GENERATED_BODY()
	
public:
	virtual void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals);
	virtual void GetInteractionBoxTitle_Implementation(FText& OutTitle);
	virtual void SetLinkedProvider_Implementation(const TScriptInterface<USPInteractionInterface>& InProvider);
	virtual TScriptInterface<USPInteractionInterface> GetLinkedProvider_Implementation() const;
	virtual void DestroySelf_Implementation();
	virtual void ProvideOnPlaced_Implementation();
	virtual void OnInteractionBoxTitleChanged_Implementation(const FText& NewTitle);
	virtual void ToggleCull_Implementation(bool IsCulled);

	UFUNCTION(BlueprintCallable)
	// Clears all points and then destroys this polygon
	void DestroyPolygon();

	UFUNCTION(BlueprintCallable)
	// Clears all points, but this polygon persists
	void ClearAllPoints();

	UFUNCTION(BlueprintCallable)
	void AddPoint(ASPPlaceablePoint* InPoint);

	UFUNCTION(BlueprintCallable)
	void CanBeClosed(bool& OutCanBeClosed) const;

	UFUNCTION(BlueprintCallable)
	FVector GetCenterpointUE() const;
	FVector GetCenterpoint() const;

	UPROPERTY(BlueprintReadWrite)
	bool IsClosed = false;

	UFUNCTION(BlueprintCallable)
	ASPPlaceablePoint* GetHead() const;

	/** Creates initial highlight polygon or updates it if it already exists */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Highlight();
	void Highlight_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UnHighlight();
	void UnHighlight_Implementation();

	UFUNCTION(BlueprintCallable)
	void Close();

	UFUNCTION(BlueprintCallable)
	void GetPointLocations(TArray<FVector>& OutLocations);

	UFUNCTION(BlueprintCallable)
	void GetActorLocations(TArray<FVector>& OutActorLocations);

	void ForEachPoint(const TFunctionRef<void(ASPPlaceablePoint*)>& Callback) const;

private:
	UFUNCTION()
	void CloseFromClickMe();

	int Length = 0;

	TSharedPtr<FPolygonListNode> Head = nullptr;

	UPROPERTY()
	TScriptInterface<USPInteractionInterface> LinkedProvider;
};
