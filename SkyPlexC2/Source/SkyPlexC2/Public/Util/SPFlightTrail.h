// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPFlightTrail.generated.h"

// Forward declarations
class UCesiumGlobeAnchorComponent;
class UProceduralMeshComponent;
class UMaterialInstance;
class UMaterialInstanceDynamic;
class ACesiumGeoreference;

UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API ASPFlightTrail : public AActor
{
	GENERATED_BODY()
	
public:
	ASPFlightTrail();

	UFUNCTION(BlueprintCallable, Category="Trail")
	void AddTrailPointCoord(const FVector& LonLatHeight);

	// set to -1 for unlimited points, 0 for none, max is 10 (unless unlimited)
	UFUNCTION(BlueprintCallable, Category="Trail")
	void SetTrailLimit(int NumPoints);

	UFUNCTION(BlueprintCallable, Category="Trail")
	void ClearTrail();

	UFUNCTION(BlueprintCallable, Category="Trail")
	void SetTrailColor(const FLinearColor& NewColor);

	// Redraws entire trail by recomputing local coordinate values from stores lon and lat points
	UFUNCTION(BlueprintCallable, Category="Trail")
	void ResetTrail();

protected:
	virtual void BeginPlay() override;

	void UpdateTrailMesh();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCesiumGlobeAnchorComponent> GlobeAnchor;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> TrailMesh;

	UPROPERTY(EditDefaultsOnly, Category="Trail")
	TObjectPtr<UMaterialInterface> TrailMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	// stored as unreal engine locations
	UPROPERTY()
	TArray<FVector> TrailPoints;

	// lon lat heights used to recompute the trail after fly to
	UPROPERTY()
	TArray<FVector> LonLatHeights;

	// half of trail width in cm
	UPROPERTY(EditDefaultsOnly, Category="Trail")
	float TrailHalfWidth = 200.0f;

	// set to -1 for unlimited points, 0 for none, max is 10 (unless unlimited)
	UPROPERTY(EditDefaultsOnly, Category="Trail")
	int TrailLimit = 10;

	FLinearColor CurrentColor = FLinearColor::Red;

	UPROPERTY()
	TObjectPtr<ACesiumGeoreference> Georeference;
};
