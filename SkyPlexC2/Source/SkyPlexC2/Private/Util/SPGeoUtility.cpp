// Copyright (c) 2025 Synetos Aerospace


#include "Util/SPGeoUtility.h"
#include "CesiumGeospatial/Ellipsoid.h"

double USPGeoUtility::GeodesicDistance(FVector LonLatHeightA, FVector LonLatHeightB) {
	const CesiumGeospatial::Ellipsoid& Ellipsoid = CesiumGeospatial::Ellipsoid::WGS84;
	return USPGeoUtility::VincentyInverseFormula(
		Ellipsoid.getMaximumRadius(),
		Ellipsoid.getMinimumRadius(),
		FMath::DegreesToRadians(LonLatHeightA.X),
		FMath::DegreesToRadians(LonLatHeightB.X),
		FMath::DegreesToRadians(LonLatHeightA.Y),
		FMath::DegreesToRadians(LonLatHeightB.Y)
	);
}

double USPGeoUtility::VincentyInverseFormula(double Major, double Minor, double Lon1, double Lon2, double Lat1, double Lat2) {
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
	double LambdaDot = SPGeoMath::TWO_PI_;

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

		Lambda = L + USPGeoUtility::ComputeDeltaLambda(Flattening, SinAlpha, CosSquaredAlpha, Sigma, SinSigma, CosSigma, CosTwiceSigmaMidpoint);
	} while (FMath::Abs(Lambda - LambdaDot) > SPGeoMath::EPSILON12);

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

double USPGeoUtility::ComputeDeltaLambda(
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