// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateboundInteractable.h"
#include "Objects/PlaceableProvider.h"
#include "Placeable.generated.h"

UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API APlaceable : public AStateboundInteractable, public IPlaceableProvider
{
	GENERATED_BODY()
	
public:
	APlaceable();

	void StartDrag() override;

	UFUNCTION(BlueprintCallable)
	void StopDrag();

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void UpdatePlacementState(bool forceValid, bool onEndOverlap, bool& invalidPlacement);

	UFUNCTION(BlueprintCallable)
	void SetOverlapEvents(bool bind, bool& invalidPlacementOnSet);

	/** If this object is being dragged */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	bool isDragging;

	/** Timer calling drag event */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	FTimerHandle dragTimerHandle;

	/** Drag offset from mouse position */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	FVector dragOffset;

	/** Whether or not this object is in a valid position */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	bool isPlacementValid;

	/** Position of object before it was dragged elsewhere*/
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Default")
	FVector posBeforeDrag;

	/** Custom event to be called when player controller is placing the actor */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CustomOnStartPlacing();
	void CustomOnStartPlacing_Implementation();

	/** Custom event to be called when player controller is finished placing the actor */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CustomOnPlaced();
	void CustomOnPlaced_Implementation();

	virtual void ProvideOnPlaced_Implementation();
protected:
	void BeginPlay() override;

	virtual void OnHover(UPrimitiveComponent* touchedComponent) override;

	virtual void OnHoverEnd(UPrimitiveComponent* touchedComponent) override;

	UFUNCTION(BlueprintCallable)
	void PlaceableOnRelease(UPrimitiveComponent* touchedComponent, FKey buttonReleased);

	UFUNCTION(BlueprintCallable)
	void PlaceableOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void PlaceableOnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Custom event to be called when an object's placement is invalid but it's not being dragged */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CustomOverlapHandler(UPARAM(ref) TArray<AActor*>& overlappingActors, bool& preventDefault);
	void CustomOverlapHandler_Implementation(UPARAM(ref) TArray<AActor*>& overlappingActors, bool& preventDefault);

	/** Custom event to be called when this actor stops overlapping other objects */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CustomOnEndOverlap();
	void CustomOnEndOverlap_Implementation();

	/** Custom event to be called before drag starts */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CustomBeforeDrag();
	void CustomBeforeDrag_Implementation();

	/** Custom event to be called when drag ends */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CustomAfterDrag();
	void CustomAfterDrag_Implementation();

	static TObjectPtr<UMaterialInstance> defaultInvalidMaterial;

private:
	void TimerEvent();
};
