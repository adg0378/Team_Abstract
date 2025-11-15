// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Core/State/SPManagerBase.h"
#include "SPSelectionManager.generated.h"

/**
 * Manages object selection
 */
UCLASS(Blueprintable)
class SKYPLEXC2_API USPSelectionManager : public USPManagerBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void AddSelected(class ASPInteractable* Actor);

	UFUNCTION(BlueprintCallable, Category = "Selection")
	void RemoveSelected(ASPInteractable* Actor);

	UFUNCTION(BlueprintCallable, Category = "Selection")
	void RefreshSelected();

	UFUNCTION(BlueprintCallable, Category = "Selection")
	void DeselectAll();

	UFUNCTION(BlueprintCallable, Category = "Selection")
	bool IsSelected() const;

	UFUNCTION(BlueprintCallable, Category = "Selection")
	bool IsMultipleSelected() const;

	UFUNCTION(BlueprintCallable, Category = "Selection")
	void DeleteSelected();

	// TODO: once UI layer is added update these in blueprint version
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateInteractionBox(const FText& Title, const TMap<FString, struct FInteractionBoxValue>& KeyVals);
	virtual void UpdateInteractionBox_Implementation(const FText& Title, const TMap<FString, FInteractionBoxValue>& KeyVals);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DeleteInteractionBox();
	virtual void DeleteInteractionBox_Implementation();

	const TArray<TObjectPtr<ASPInteractable>>& GetSelectedActors() const;

	void CheckMouseLocationForDeselect(FVector& WorldLocation, FVector& WorldDirection);

protected:
	UFUNCTION(BlueprintCallable)
	void CompileInteractionBoxKeyVals(TMap<FString, FInteractionBoxValue>& outKeyVals);

	UFUNCTION(BlueprintCallable)
	FText CompileInteractionBoxTitle();

private:
	UPROPERTY()
	TArray<TObjectPtr<ASPInteractable>> SelectedActors;
};
