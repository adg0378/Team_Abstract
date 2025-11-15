// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/SPInteractable.h"
#include "Objects/SPPlaceableInterface.h"
#include "SPPlaceable.generated.h"

UCLASS(Blueprintable)
class SKYPLEXC2_API ASPPlaceable : public ASPInteractable, public ISPPlaceableInterface
{
	GENERATED_BODY()
	
public:
	ASPPlaceable();

	void StartDrag() override;

	UFUNCTION(BlueprintCallable)
	void StopDrag();

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void UpdatePlacementState(bool ForceValid, bool OnEndOverlap, bool& InvalidPlacement);

	UFUNCTION(BlueprintCallable)
	void SetOverlapEvents(bool Bind, bool& InvalidPlacementOnSet);

	UFUNCTION(BlueprintCallable)
	bool IsDragging() const;

	UFUNCTION(BlueprintCallable)
	bool IsPlacementValid() const;

	/** Custom event to be called when player controller is placing the actor */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CustomOnStartPlacing();
	void CustomOnStartPlacing_Implementation();

	/** Custom event to be called when player controller is finished placing the actor */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CustomOnPlaced();
	void CustomOnPlaced_Implementation();

	void SetToPosBeforeDrag();

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
	void CustomOverlapHandler(UPARAM(ref) TArray<AActor*>& OverlappingActors, bool& PreventDefault);
	void CustomOverlapHandler_Implementation(UPARAM(ref) TArray<AActor*>& OverlappingActors, bool& PreventDefault);

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

	UPROPERTY()
	TObjectPtr<UMaterialInstance> DefaultInvalidMaterial;

	bool _IsDragging = false;

	bool _IsPlacementValid = true;

private:
	UPROPERTY()
	FTimerHandle DragTimerHandle;

	/** Drag offset from mouse position */
	FVector DragOffset = FVector::ZeroVector;

	/** Position of object before it was dragged elsewhere*/
	FVector PosBeforeDrag = FVector::ZeroVector;

	void TimerEvent();
};
