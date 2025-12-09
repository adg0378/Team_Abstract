// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CesiumGeospatial/Ellipsoid.h"
#include "GeographicUtility.generated.h"

// forward declarations
class ACesiumGeoreference;

namespace GeographicMath {
	constexpr double TWO_PI_ = 6.28318530717958647692;
	constexpr double EPSILON12 = 1e-12;
	constexpr int METERS_PER_LAT = 111139;

	/* This is a rough approximation. True values must be computed based on latitude. */
	constexpr int METERS_PER_LON = 111320;
}

/** Stores a latitude longitude bounding box */
USTRUCT(BlueprintType)
struct FLatLonBoundingBox
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float minLat;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float maxLat;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float minLon;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float maxLon;
};

/**
 * Utility functions for geographic calculations
 */
UCLASS(BlueprintType, Blueprintable)
class SKYPLEXC2_API UGeographicUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static void GetBoundingBox(float lat, float lon, float radiusNM, FLatLonBoundingBox& outBox);

	static FVector DVec3ToFVector(const glm::dvec3& dVec);
	
	/** Call's VincentyInverseFormula with a WGS84 ellipsoid. Distance is in meters. */
	UFUNCTION(BlueprintCallable)
	static double GeodesicDistance(FVector LonLatHeightA, FVector LonLatHeightB);

	// Lon and lats must be in radians, not degrees
	static double VincentyInverseFormula(double Major, double Minor, double Lon1, double Lon2, double Lat1, double Lat2);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Geographic")
	static const double MetersFromLatitude(double Latitude, double Meters);

	/* Setting latitude to -999 will return a faster but less-accurate value */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category="Geographic")
	static const double MetersFromLongitude(double Longitude, double Meters, double Latitude = -999.0f);

	static FVector LatLonToLocal(
		ACesiumGeoreference* Georeference,
		const FVector& OriginLonLatHeight,
		const FVector& PointLonLatHeight
	);

	static TArray<FVector> LatLonToLocal(
		ACesiumGeoreference* Georeference,
		const FVector& OriginLonLatHeight,
		const TArray<FVector>& PointLonLatHeights
	);

	static FVector LocalToLatLon(
		ACesiumGeoreference* Georeference,
		const FVector& OriginLonLatHeight,
		const FVector& PointLocal
	);

	static TArray<FVector> LocalToLatLon(
		ACesiumGeoreference* Georeference,
		const FVector& OriginLonLatHeight,
		const TArray<FVector>& PointLocals
	);

	static TArray<FVector> RotatePoints(
		const TArray<FVector>& LocalPoints,
		double AngleDeg
	);

	static TArray<FVector2D> GetPolygonLineIntersections(
		const TArray<FVector>& PolyPointsLocal,
		double LineY
	);

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
