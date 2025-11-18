// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Objects/Interests/SPInterest.h"
#include "SPGeoFence.generated.h"

/**
 * Geo Fence
 */
UCLASS()
class SKYPLEXC2_API USPGeoFence : public USPInterest
{
	GENERATED_BODY()
	
public:
	virtual EInterestType GetInterestType() const override;

	virtual void SetSelected(bool IsSelected);

	virtual void SetName(FString InName) override;

	UFUNCTION(BlueprintCallable)
	void SetPolygon(class ASPPolygon* InPolygon);

	UFUNCTION(BlueprintCallable)
	ASPPolygon* GetPolygon() const;

	virtual void DestroySelf_Implementation();

	virtual void ToggleCull_Implementation(bool IsCulled) override;

private:
	UPROPERTY()
	TObjectPtr<ASPPolygon> Polygon;

};
