// Copyright (c) 2025 Synetos Aerospace


#include "GeographicUtility.h"
#include <CesiumGeoreference.h>
#include <CesiumEllipsoid.h>
#include <CesiumGeospatial/LocalHorizontalCoordinateSystem.h>

// If we call this a variation of pi it breaks the MAC build so yay fun variable names
const double poop = 3.14159265358979323846;

void UGeographicUtility::GetBoundingBox(float lat, float lon, float radiusNM, FLatLonBoundingBox& outBox) {
	float latDeg = radiusNM / 60.0;
	float lonDeg = radiusNM / (60.0 * cos(lat * poop / 180.0));
	outBox = { lat - latDeg, lat + latDeg, lon - lonDeg, lon + lonDeg };
}

FVector UGeographicUtility::DVec3ToFVector(const glm::dvec3& dVec) {
	return FVector(dVec.x, dVec.y, dVec.z);
}

double UGeographicUtility::GeodesicDistance(FVector LonLatHeightA, FVector LonLatHeightB) {
	const CesiumGeospatial::Ellipsoid& Ellipsoid = CesiumGeospatial::Ellipsoid::WGS84;
	return UGeographicUtility::VincentyInverseFormula(
		Ellipsoid.getMaximumRadius(),
		Ellipsoid.getMinimumRadius(),
		FMath::DegreesToRadians(LonLatHeightA.X),
		FMath::DegreesToRadians(LonLatHeightB.X),
		FMath::DegreesToRadians(LonLatHeightA.Y),
		FMath::DegreesToRadians(LonLatHeightB.Y)
	);
}

double UGeographicUtility::VincentyInverseFormula(double Major, double Minor, double Lon1, double Lon2, double Lat1, double Lat2) {
	double Flattening = (Major - Minor) / Major;
	double L = Lon1 - Lon2;

	double U1 = FMath::Atan((1 - Flattening) * FMath::Tan(Lat1));
	double U2 = FMath::Atan((1 - Flattening) * FMath::Tan(Lat2));

	double CosU1 = FMath::Cos(U1);
	double SinU1 = FMath::Sin(U1);
	double CosU2 = FMath::Cos(U2);
	double SinU2 = FMath::Sin(U2);

	double CosCos = CosU1 * CosU2;
	double CosSin = CosU1 * SinU2;
	double SinSin = SinU1 * SinU2;
	double SinCos = SinU1 * CosU2;

	double Lambda = L;
	double LambdaDot = GeographicMath::TWO_PI_;

	double CosLambda = FMath::Cos(Lambda);
	double SinLambda = FMath::Sin(Lambda);

	double Sigma;
	double CosSigma;
	double SinSigma;
	double CosSquaredAlpha;
	double CosTwiceSigmaMidpoint;

	do {
		CosLambda = FMath::Cos(Lambda);
		SinLambda = FMath::Sin(Lambda);

		double Temp = CosSin - SinCos * CosLambda;
		SinSigma = FMath::Sqrt(CosU2 * CosU2 * SinLambda * SinLambda + Temp * Temp);
		CosSigma = SinSin + CosCos * CosLambda;

		Sigma = FMath::Atan2(SinSigma, CosSigma);

		double SinAlpha;

		if (SinSigma == 0.0) {
			SinAlpha = 0.0;
			CosSquaredAlpha = 1.0;
		}
		else {
			SinAlpha = (CosCos * SinLambda) / SinSigma;
			CosSquaredAlpha = 1.0 - SinAlpha * SinAlpha;
		}

		LambdaDot = Lambda;

		CosTwiceSigmaMidpoint = CosSigma - (2.0 * SinSin) / CosSquaredAlpha;

		if (!FMath::IsFinite(CosTwiceSigmaMidpoint)) {
			CosTwiceSigmaMidpoint = 0.0;
		}

		Lambda = L + UGeographicUtility::ComputeDeltaLambda(Flattening, SinAlpha, CosSquaredAlpha, Sigma, SinSigma, CosSigma, CosTwiceSigmaMidpoint);
	} while (FMath::Abs(Lambda - LambdaDot) > GeographicMath::EPSILON12);

	double USquared = (CosSquaredAlpha * (Major * Major - Minor * Minor)) / (Minor * Minor);
	double A = 1.0 +
		(USquared *
			(4096.0 + USquared * (USquared * (320.0 - 175.0 * USquared) - 768.0))) /
		16384.0;
	double B = 
		(USquared *
			(256.0 + USquared * (USquared * (74.0 - 47.0 * USquared) - 128.0))) /
		1024.0;
	double CosSquaredTwiceSigmaMidpoint = CosTwiceSigmaMidpoint * CosTwiceSigmaMidpoint;
	double DeltaSigma =
		B *
		SinSigma *
		(CosTwiceSigmaMidpoint +
			(B *
				(CosSigma * (2.0 * CosSquaredTwiceSigmaMidpoint - 1.0) -
					(B *
						CosTwiceSigmaMidpoint *
						(4.0 * SinSigma * SinSigma - 3.0) *
						(4.0 * CosSquaredTwiceSigmaMidpoint - 3.0)) /
					6.0)) /
			4.0);
	double Distance = Minor * A * (Sigma - DeltaSigma);
	return Distance;
}

double UGeographicUtility::ComputeDeltaLambda(
	const double& Flattening,
	const double& SinAlpha,
	const double& CosSquaredAlpha,
	const double& Sigma,
	const double& SinSigma,
	const double& CosSigma,
	const double& CosTwiceSigmaMidpoint
) {
	double C = (Flattening * CosSquaredAlpha * (4.0 + Flattening * (4.0 - 3.0 * CosSquaredAlpha))) / 16.0;
	return (1.0 - C) *
		Flattening *
		SinAlpha *
		(Sigma +
			C *
			SinSigma *
			(CosTwiceSigmaMidpoint +
				C *
				CosSigma *
				(2.0 * CosTwiceSigmaMidpoint * CosTwiceSigmaMidpoint - 1.0)));
}

const double UGeographicUtility::MetersFromLatitude(double Latitude, double Meters) {
	UE_LOG(LogTemp, Warning, TEXT("M FROM LAT: og=%f ---- new=%f"), Latitude, Latitude + (Meters / GeographicMath::METERS_PER_LAT))
	return Latitude + (Meters / GeographicMath::METERS_PER_LAT);
}

const double UGeographicUtility::MetersFromLongitude(double Longitude, double Meters, double Latitude) {
	if (Latitude == -999.0f) {
		return Longitude + (Meters / GeographicMath::METERS_PER_LON);
	}
	UE_LOG(LogTemp, Warning, TEXT("M FROM LAT: og=%f ---- new=%f"), Longitude, Longitude + (Meters / (GeographicMath::METERS_PER_LON * FMath::Cos(Latitude))))
	return Longitude + (Meters / (GeographicMath::METERS_PER_LON * FMath::Cos(Latitude)));
}

FVector UGeographicUtility::LatLonToLocal(
	ACesiumGeoreference* Georeference,
	const FVector& OriginLonLatHeight,
	const FVector& PointLonLatHeight
) {
	UCesiumEllipsoid* Ellipsoid = Georeference->GetEllipsoid();
	FVector ECEFOrigin = Ellipsoid->LongitudeLatitudeHeightToEllipsoidCenteredEllipsoidFixed(OriginLonLatHeight);
	FVector ECEFPoint = Ellipsoid->LongitudeLatitudeHeightToEllipsoidCenteredEllipsoidFixed(PointLonLatHeight);
	
	FVector Diff = ECEFPoint - ECEFOrigin;
	double LatRad = FMath::DegreesToRadians(OriginLonLatHeight.Y);
	double LonRad = FMath::DegreesToRadians(OriginLonLatHeight.X);

	FVector East = FVector(-FMath::Sin(LonRad), FMath::Cos(LonRad), 0.0);
	FVector North = FVector(-FMath::Sin(LatRad) * FMath::Cos(LonRad), -FMath::Sin(LatRad) * FMath::Sin(LonRad), FMath::Cos(LatRad));
	FVector Up = FVector(FMath::Cos(LatRad) * FMath::Cos(LonRad), FMath::Cos(LatRad) * FMath::Sin(LonRad), FMath::Sin(LatRad));

	double x = Diff.Dot(East);
	double y = Diff.Dot(North);
	double z = Diff.Dot(Up);

	return FVector(x, y, z);
}

TArray<FVector> UGeographicUtility::LatLonToLocal(
	ACesiumGeoreference* Georeference,
	const FVector& OriginLonLatHeight,
	const TArray<FVector>& PointLonLatHeights
) {
	UCesiumEllipsoid* Ellipsoid = Georeference->GetEllipsoid();
	FVector ECEFOrigin = Ellipsoid->LongitudeLatitudeHeightToEllipsoidCenteredEllipsoidFixed(OriginLonLatHeight);

	double LatRad = FMath::DegreesToRadians(OriginLonLatHeight.Y);
	double LonRad = FMath::DegreesToRadians(OriginLonLatHeight.X);

	FVector East = FVector(-FMath::Sin(LonRad), FMath::Cos(LonRad), 0.0);
	FVector North = FVector(-FMath::Sin(LatRad) * FMath::Cos(LonRad), -FMath::Sin(LatRad) * FMath::Sin(LonRad), FMath::Cos(LatRad));
	FVector Up = FVector(FMath::Cos(LatRad) * FMath::Cos(LonRad), FMath::Cos(LatRad) * FMath::Sin(LonRad), FMath::Sin(LatRad));

	TArray<FVector> LocalPoints;
	LocalPoints.Reserve(PointLonLatHeights.Num());

	for (const FVector& Point : PointLonLatHeights) {
		FVector ECEFPoint = Ellipsoid->LongitudeLatitudeHeightToEllipsoidCenteredEllipsoidFixed(Point);
		FVector Diff = ECEFPoint - ECEFOrigin;

		double x = Diff.Dot(East);
		double y = Diff.Dot(North);
		double z = Diff.Dot(Up);

		LocalPoints.Push(FVector(x, y, z));
	}

	return LocalPoints;
}

TArray<FVector> UGeographicUtility::RotatePoints(
	const TArray<FVector>& LocalPoints,
	double AngleDeg
) {
	double A = FMath::DegreesToRadians(AngleDeg);
	double CosA = FMath::Cos(A);
	double SinA = FMath::Sin(A);

	TArray<FVector> Res;
	Res.Reserve(LocalPoints.Num());

	for (auto& Point : LocalPoints) {
		double X = Point.X * CosA - Point.Y * SinA;
		double Y = Point.X * SinA + Point.Y * CosA;
		Res.Add(FVector(X, Y, 0.0f));
	}
	return Res;
}

FVector UGeographicUtility::LocalToLatLon(
	ACesiumGeoreference* Georeference,
	const FVector& OriginLonLatHeight,
	const FVector& PointLocal
) {
	double LatRad = FMath::DegreesToRadians(OriginLonLatHeight.Y);
	double LonRad = FMath::DegreesToRadians(OriginLonLatHeight.X);

	FVector East = FVector(-FMath::Sin(LonRad), FMath::Cos(LonRad), 0.0);
	FVector North = FVector(-FMath::Sin(LatRad) * FMath::Cos(LonRad), -FMath::Sin(LatRad) * FMath::Sin(LonRad), FMath::Cos(LatRad));
	FVector Up = FVector(FMath::Cos(LatRad) * FMath::Cos(LonRad), FMath::Cos(LatRad) * FMath::Sin(LonRad), FMath::Sin(LatRad));

	UCesiumEllipsoid* Ellipsoid = Georeference->GetEllipsoid();
	FVector ECEFOrigin = Ellipsoid->LongitudeLatitudeHeightToEllipsoidCenteredEllipsoidFixed(OriginLonLatHeight);

	FVector ECEFPoint = ECEFOrigin + PointLocal.X * East + PointLocal.Y * North + PointLocal.Z * Up;

	FVector LonLatHeight = Ellipsoid->EllipsoidCenteredEllipsoidFixedToLongitudeLatitudeHeight(ECEFPoint);
	return LonLatHeight;
}

TArray<FVector> UGeographicUtility::LocalToLatLon(
	ACesiumGeoreference* Georeference,
	const FVector& OriginLonLatHeight,
	const TArray<FVector>& PointLocals
) {
	double LatRad = FMath::DegreesToRadians(OriginLonLatHeight.Y);
	double LonRad = FMath::DegreesToRadians(OriginLonLatHeight.X);

	FVector East = FVector(-FMath::Sin(LonRad), FMath::Cos(LonRad), 0.0);
	FVector North = FVector(-FMath::Sin(LatRad) * FMath::Cos(LonRad), -FMath::Sin(LatRad) * FMath::Sin(LonRad), FMath::Cos(LatRad));
	FVector Up = FVector(FMath::Cos(LatRad) * FMath::Cos(LonRad), FMath::Cos(LatRad) * FMath::Sin(LonRad), FMath::Sin(LatRad));

	UCesiumEllipsoid* Ellipsoid = Georeference->GetEllipsoid();
	FVector ECEFOrigin = Ellipsoid->LongitudeLatitudeHeightToEllipsoidCenteredEllipsoidFixed(OriginLonLatHeight);

	TArray<FVector> Res;
	Res.Reserve(PointLocals.Num());

	for (const auto& Point : PointLocals) {
		FVector ECEFPoint = ECEFOrigin + Point.X * East + Point.Y * North + Point.Z * Up;
		FVector LonLatHeight = Ellipsoid->EllipsoidCenteredEllipsoidFixedToLongitudeLatitudeHeight(ECEFPoint);
		Res.Add(LonLatHeight);
	}

	return Res;
}

TArray<FVector2D> UGeographicUtility::GetPolygonLineIntersections(
	const TArray<FVector>& PolyPointsLocal,
	double LineY
) {
	TArray<FVector2D> Intersections;

	int N = PolyPointsLocal.Num();
	for (int i = 0; i < N; ++i) {
		FVector P1 = PolyPointsLocal[i];
		FVector P2 = PolyPointsLocal[(i + 1) % N];

		if (P1.Y <= LineY && P2.Y >= LineY || P1.Y >= LineY && P2.Y <= LineY) {
			float t = (LineY - P1.Y) / (P2.Y - P1.Y);
			float X = P1.X + t * (P2.X - P1.X);
			Intersections.Add(FVector2D(X, LineY));
		}
	}

	Intersections.Sort([](const FVector2D& A, const FVector2D& B) { return A.X < B.X; });
	return Intersections;
}
