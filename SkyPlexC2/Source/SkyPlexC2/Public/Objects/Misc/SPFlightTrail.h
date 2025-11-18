// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPFlightTrail.generated.h"

UCLASS()
class SKYPLEXC2_API ASPFlightTrail : public AActor
{
	GENERATED_BODY()
	
public:
	ASPFlightTrail();

	UFUNCTION(BlueprintCallable, Category = "Trail")
	void AddTrailPointCoord(const FVector& LonLatHeight);

	// set to -1 for unlimited points, 0 for none, max is 10 (unless unlimited)
	UFUNCTION(BlueprintCallable, Category = "Trail")
	void SetTrailLimit(int NumPoints);

	UFUNCTION(BlueprintCallable, Category = "Trail")
	void ClearTrail();

	UFUNCTION(BlueprintCallable, Category = "Trail")
	void SetTrailColor(const FLinearColor& NewColor);

protected:
	virtual void BeginPlay() override;

	void UpdateTrailMesh();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UCesiumGlobeAnchorComponent> GlobeAnchorComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UProceduralMeshComponent> TrailMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Trail")
	TObjectPtr<UMaterialInterface> TrailMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	// stored as lon lat height, converted in UpdateTrailMesh
	UPROPERTY()
	TArray<FVector> TrailPoints;

	// half of trail width in cm
	UPROPERTY(EditDefaultsOnly, Category = "Trail")
	float TrailHalfWidth = 200.0f;

	// set to -1 for unlimited points, 0 for none, max is 10 (unless unlimited)
	UPROPERTY(EditDefaultsOnly, Category = "Trail")
	int TrailLimit = 10;

	FLinearColor CurrentColor = FLinearColor::Red;

	UPROPERTY()
	TObjectPtr<class ACesiumGeoreference> Georeference;
};
