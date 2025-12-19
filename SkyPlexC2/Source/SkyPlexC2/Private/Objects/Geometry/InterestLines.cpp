// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Geometry/InterestLines.h"

// Sets default values
AInterestLines::AInterestLines() {
	DrawingSpline = CreateDefaultSubobject<USplineComponent>(TEXT("MyDrawingSpline"));
	SetRootComponent(DrawingSpline);
}

// Called when the game starts or when spawned
void AInterestLines::BeginPlay() {
	Super::BeginPlay();
	DrawingSpline->ClearSplinePoints();
}

void AInterestLines::DestroyInterestLines() {
	DrawingSpline->ClearSplinePoints();
	Destroy();
}

void AInterestLines::SpawnInterestLines_Implementation() {

}

void AInterestLines::DrawLinesViaActorLocations(TArray<FVector> ActorPoints, FString tag) {
	int index = 0;
	for (const FVector& point : ActorPoints) {
		DrawingSpline->AddSplinePoint(point, ESplineCoordinateSpace::World, true);
		DrawingSpline->SetSplinePointType(index, ESplinePointType::CurveClamped, true);
		DrawingSpline->SetTangentsAtSplinePoint(index,
			FVector(0, 0, 0), FVector(0, 0, 0), ESplineCoordinateSpace::World, true);

		index++;
	}
	DrawingSpline->SetClosedLoop(true, true);

	if (!tag.IsEmpty()) {
		SetLineColorViaTag(tag);
	}
	SpawnInterestLines();
}

void AInterestLines::DrawLinesViaLonLatHgts(TArray<FVector> LonLatHgts, FString tag) {
	UWorld* World = GetWorld();
	ACesiumGeoreference* Georeference = ACesiumGeoreference::GetDefaultGeoreference(World);

	int index = 0;
	for (const FVector& LonLatHgt : LonLatHgts) {
		FVector unrealCords = Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(LonLatHgt);
		DrawingSpline->AddSplinePoint(unrealCords, ESplineCoordinateSpace::World, true);
		DrawingSpline->SetSplinePointType(index, ESplinePointType::CurveClamped, true);
		DrawingSpline->SetTangentsAtSplinePoint(index,
			FVector(0, 0, 0), FVector(0, 0, 0), ESplineCoordinateSpace::World, true);

		index++;
	}
	DrawingSpline->SetClosedLoop(true, true);

	if (!tag.IsEmpty()) {
		SetLineColorViaTag(tag);
	}
	SpawnInterestLines();
}

void AInterestLines::SetLineColorViaTag_Implementation(const FString& tag) {

}

TArray<FVector> AInterestLines::GetLineLonLatHgts() {
	TArray<FVector> lonLatHgts;
	for (size_t i = 0; i < DrawingSpline->GetNumberOfSplinePoints(); i++) {
		lonLatHgts.Add(DrawingSpline->GetWorldLocationAtSplinePoint(i));
	}

	return lonLatHgts;
}