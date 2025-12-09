#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Objects/InteractionBoxProvider.h"
#include "Interactable.generated.h"

// Forward declarations
class UCesiumGlobeAnchorComponent;

UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API AInteractable : public AActor, public IInteractionBoxProvider
{
	GENERATED_BODY()
	
public:
	AInteractable();

	/** Overriden by placeable subclasses **/
	UFUNCTION(BlueprintCallable)
	virtual void StartDrag();

	UFUNCTION(BlueprintCallable)
	void Select();

	UFUNCTION(BlueprintCallable)
	void Deselect(bool deleteFromGameStateSelected = true, bool unhighlight = true);


	virtual void GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals);
	virtual void GetInteractionBoxTitle_Implementation(FText& OutTitle);
	virtual void SetLinkedProvider_Implementation(const TScriptInterface<UInteractionBoxProvider>& InProvider);
	virtual void GetLinkedProvider_Implementation(TScriptInterface<UInteractionBoxProvider>& OutProvider) const;
	virtual void DestroySelf_Implementation();
	virtual void OnInteractionBoxTitleChanged_Implementation(const FText &NewTitle);
	virtual void ToggleCull_Implementation(bool IsCulled, bool FromFlyTo);

	UFUNCTION(BlueprintCallable)
	void GetLongitudeLatitudeHeight(FVector& OutLonLatHeight) const;

	UFUNCTION(BlueprintCallable)
	void MoveToLongitudeLatitudeHeight(FVector InLonLatHeight) const;

	/** Cesium globe anchor component */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UCesiumGlobeAnchorComponent> cesiumGlobeAnchor;

	/** Overriden by subclasses */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Default")
	TObjectPtr<UStaticMeshComponent> staticMesh;

	/** Set by subclasses if it should be draggable by the user */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	bool userPlacementEnabled;

	/** Check whether object is selected */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	bool isSelected;

protected:
	/** Ensure subclass's static mesh is set before calling this method */
	virtual void BeginPlay() override;

	/** Set by subclasses to determine default material */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TObjectPtr<UMaterialInstance> defaultMaterial;

	/** Selected material */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
	TObjectPtr<UMaterialInstance> selectedMaterial;

	UPROPERTY()
	TScriptInterface<UInteractionBoxProvider> LinkedProvider;

	UFUNCTION(BlueprintCallable)
	virtual void OnHover(UPrimitiveComponent* touchedComponent);

	UFUNCTION(BlueprintCallable)
	virtual void OnHoverEnd(UPrimitiveComponent* touchedComponent);

	UFUNCTION(BlueprintCallable)
	virtual void OnClick(UPrimitiveComponent* touchedComponent, FKey buttonPressed);
};

