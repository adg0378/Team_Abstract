// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SPGeoUtility.generated.h"

namespace SPGeoMath {
	constexpr double TWO_PI_ = 6.28318530717958647692;
	constexpr double EPSILON12 = 1e-12;
	constexpr int METERS_PER_LAT = 111139;

	/* This is a rough approximation. True values must be computed based on latitude. */
	constexpr int METERS_PER_LON = 111320;
}

/**
 * Geographic utility functions not yet provided by Cesium for Unreal
 */
UCLASS()
class SKYPLEXC2_API USPGeoUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Call's VincentyInverseFormula with a WGS84 ellipsoid. Distance is in meters. */
	UFUNCTION(BlueprintCallable)
	static double GeodesicDistance(FVector LonLatHeightA, FVector LonLatHeightB);

	// Lon and lats must be in radians, not degrees
	static double VincentyInverseFormula(double Major, double Minor, double Lon1, double Lon2, double Lat1, double Lat2);

private:
	static double ComputeDeltaLambda(
		const double& Flattening,
		const double& SinAlpha,
		const double& CosSquaredAlpha,
		const double& Sigma,
		const double& SinSigma,
		const double& CosSigma,
		const double& CosTwiceSigmaMidpoint
	);
};
