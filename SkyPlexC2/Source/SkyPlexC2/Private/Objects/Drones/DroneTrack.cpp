// Copyright (c) 2025 Synetos Aerospace

#include "Objects/Drones/DroneTrack.h"
#include "CesiumGeoreference.h"
#include <CesiumGeospatial/Cartographic.h>
#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumGlobeAnchorComponent.h>
#include <CesiumGeoreference.h>

// Sets default values
ADroneTrack::ADroneTrack()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize the track
	SplineTrack = CreateDefaultSubobject<USplineComponent>(TEXT("SplineTrack"));
	// This lets us visualize the spline in Play mode
	SplineTrack->SetDrawDebug(true);

	// Set the color of the spline
	SplineTrack->SetUnselectedSplineSegmentColor(FLinearColor(1.f, 0.f, 0.f));

    CesiumGeoreference = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
}

void ADroneTrack::DestroyTrack() {
    ClearSplineTrackPoints();
    Destroy();
}

// Called when the game starts or when spawned
void ADroneTrack::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADroneTrack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADroneTrack::LoadSplineTrackPoints()
{
    if (this->AircraftsRawDataTable != nullptr && this->CesiumGeoreference != nullptr)
    {
        int32 PointIndex = 0;
        for (auto& row : this->AircraftsRawDataTable->GetRowMap())
        {
            FAircraftRawData* Point = (FAircraftRawData*)row.Value;
            // Get row data point in lat/long/alt and transform it into UE points
            double PointLatitude = Point->Latitude;
            double PointLongitude = Point->Longitude;
            double PointHeight = Point->Height;

            // Compute the position in UE coordinates
            FVector SplinePointPosition = this->CesiumGeoreference->TransformLongitudeLatitudeHeightPositionToUnreal(FVector(PointLongitude, PointLatitude, PointHeight));
            this->SplineTrack->AddSplinePointAtIndex(SplinePointPosition, PointIndex, ESplineCoordinateSpace::World, false);

            // Get the up vector at the position to orient the aircraft
            const CesiumGeospatial::Ellipsoid& Ellipsoid = CesiumGeospatial::Ellipsoid::WGS84;
            glm::dvec3 upVector = Ellipsoid.geodeticSurfaceNormal(CesiumGeospatial::Cartographic(FMath::DegreesToRadians(PointLongitude),
                FMath::DegreesToRadians(PointLatitude),
                FMath::DegreesToRadians(PointHeight)));

            // Compute the up vector at each point to correctly orient the plane 
            FVector4 ecefUp(upVector.x, upVector.y, upVector.z, 0.0);
            FMatrix ecefToUnreal = this->CesiumGeoreference->ComputeEarthCenteredEarthFixedToUnrealTransformation();			  
            FVector4 unrealUp = ecefToUnreal.TransformFVector4(ecefUp);
            this->SplineTrack->SetUpVectorAtSplinePoint(PointIndex, FVector(unrealUp), ESplineCoordinateSpace::World, false);

            PointIndex++;
        }
        
        PointCount = PointIndex;
        this->SplineTrack->UpdateSpline();
    }
}

void ADroneTrack::LoadTrackByCSVFile(FString FilePath) {
    FString CSVstring;
    FFileHelper::LoadFileToString(CSVstring, *FilePath);

    AircraftsRawDataTable = NewObject<UDataTable>(GetTransientPackage(), TEXT("DroneTrackDataTable"));
    AircraftsRawDataTable->RowStruct = FAircraftRawData::StaticStruct();

    TArray<FString> ErrorLog = AircraftsRawDataTable->CreateTableFromCSVString(CSVstring);

    if (ErrorLog.Num() == 0) {
        UE_LOG(LogTemp, Log, TEXT("AircraftRawData created data table from CSV file."));
    
    } else {
        for (const FString& Error : ErrorLog)
        {
            UE_LOG(LogTemp, Warning, TEXT("CSV import problem: %s"), *Error);
        }
    }
}


void ADroneTrack::ClearSplineTrackPoints() {
    this->SplineTrack->ClearSplinePoints(true);
}

int32 ADroneTrack::GetPointCount() {
    return PointCount;
}
