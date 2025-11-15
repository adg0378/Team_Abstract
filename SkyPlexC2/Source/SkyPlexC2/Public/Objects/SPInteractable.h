// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Objects/SPInteractionInterface.h"
#include "SPInteractable.generated.h"

UCLASS(Blueprintable)
class SKYPLEXC2_API ASPInteractable : public AActor, public ISPInteractionInterface
{
	GENERATED_BODY()
	
public:	
	ASPInteractable();

	/* Overriden by placeable subclasses */
	UFUNCTION(BlueprintCallable)
	virtual void StartDrag();

	UFUNCTION(BlueprintCallable)
	void Select();

	UFUNCTION(BlueprintCallable)
	void Deselect(bool DeleteFromGameStateSelected = true, bool Unhighlight = true);

	virtual void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals);
	virtual void GetInteractionBoxTitle_Implementation(FText& OutTitle);
	virtual void SetLinkedProvider_Implementation(const TScriptInterface<USPInteractionInterface>& InProvider);
	virtual TScriptInterface<USPInteractionInterface> GetLinkedProvider_Implementation() const;
	virtual void DestroySelf_Implementation();
	virtual void OnInteractionBoxTitleChanged_Implementation(const FText& NewTitle);
	virtual void ToggleCull_Implementation(bool IsCulled);

	UFUNCTION(BlueprintCallable)
	FVector GetLongitudeLatitudeHeight() const;

	UFUNCTION(BlueprintCallable)
	void MoveToLongitudeLatitudeHeight(FVector InLonLatHeight) const;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<class UCesiumGlobeAnchorComponent> GlobeAnchorComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	/** Set by subclasses if it should be draggable by the user */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool UserPlacementEnabled = false;

	/** Check whether object is selected */
	UFUNCTION(BlueprintCallable)
	bool IsSelected() const;

protected:
	virtual void BeginPlay() override;

	/** Set by subclasses to determine default material */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TObjectPtr<UMaterialInstance> DefaultMaterial;

	/** Selected material */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TObjectPtr<UMaterialInstance> SelectedMaterial;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY()
	TScriptInterface<USPInteractionInterface> LinkedProvider;

	UFUNCTION(BlueprintCallable)
	virtual void OnHover(UPrimitiveComponent* touchedComponent);

	UFUNCTION(BlueprintCallable)
	virtual void OnHoverEnd(UPrimitiveComponent* touchedComponent);

	UFUNCTION(BlueprintCallable)
	virtual void OnClick(UPrimitiveComponent* touchedComponent, FKey buttonPressed);

	bool _IsSelected = false;
};
