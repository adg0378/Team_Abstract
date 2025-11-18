// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Misc/SPFlightTrail.h"
#include "ProceduralMeshComponent.h"
#include "CesiumGlobeAnchorComponent.h"
#include "CesiumGeoreference.h"
#include "Core/State/SPGameState.h"

// Sets default values
ASPFlightTrail::ASPFlightTrail()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = AActor::CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	GetRootComponent()->SetMobility(EComponentMobility::Movable);

	GlobeAnchorComponent = CreateDefaultSubobject<UCesiumGlobeAnchorComponent>(TEXT("GlobeAnchor"));
	GlobeAnchorComponent->bAutoActivate = true;

	TrailMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TrailMesh"));
	TrailMesh->SetupAttachment(RootComponent);
	TrailMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TrailMesh->SetGenerateOverlapEvents(false);
	TrailMesh->CastShadow = false;
	TrailMesh->bUseComplexAsSimpleCollision = false;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("Material'/Game/Materials/M_Trail.M_Trail'"));
	if (MaterialFinder.Succeeded()) {
		TrailMaterial = MaterialFinder.Object;
	}

	CurrentColor = FLinearColor::Red;
}

// Called when the game starts or when spawned
void ASPFlightTrail::BeginPlay()
{
	Super::BeginPlay();

	ASPGameState* GameState = ASPGameState::GetSPGameState(this);

	Georeference = GameState->CesiumGeoreference;

	if (TrailMaterial) {
		DynamicMaterial = UMaterialInstanceDynamic::Create(TrailMaterial, this);
		TrailMesh->SetMaterial(0, DynamicMaterial);
		DynamicMaterial->SetVectorParameterValue("Color", CurrentColor);
	}
}

void ASPFlightTrail::SetTrailColor(const FLinearColor& NewColor) {
	CurrentColor = NewColor;

	if (DynamicMaterial) {
		DynamicMaterial->SetVectorParameterValue("Color", NewColor);
	}
}

void ASPFlightTrail::SetTrailLimit(int NumPoints) {
	if (NumPoints > 10) {
		NumPoints = 10;
	}
	else if (NumPoints < -1) {
		NumPoints = -1;
	}

	TrailLimit = NumPoints;

	if (TrailLimit == 0) {
		ClearTrail();
	}
	if (TrailLimit > 0 && TrailPoints.Num() > TrailLimit) {
		TArray<FVector> NewPoints;
		for (int i = TrailPoints.Num() - TrailLimit - 1; i < TrailPoints.Num(); ++i) {
			NewPoints.Add(TrailPoints[i]);
		}
		TrailPoints = NewPoints;
	}
}

void ASPFlightTrail::ClearTrail() {
	TrailPoints.Empty();
	TrailMesh->ClearAllMeshSections();
}

void ASPFlightTrail::AddTrailPointCoord(const FVector& LonLatHeight) {
	if (TrailLimit == 0) {
		return;
	}

	if (TrailPoints.Num() == 0) {
		GlobeAnchorComponent->MoveToLongitudeLatitudeHeight(LonLatHeight);
	}

	if (TrailLimit > 0 && TrailPoints.Num() + 1 > TrailLimit) {
		TrailPoints.RemoveAt(0, EAllowShrinking::No);
	}

	TrailPoints.Add(Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(LonLatHeight) - GetActorLocation());
	// DrawDebugPoint(GetWorld(), WorldLocation, 10.0f, FColor::Red, false, 5.0f);
	UpdateTrailMesh();
}

void ASPFlightTrail::UpdateTrailMesh() {
	int NumPoints = TrailPoints.Num();
	if (NumPoints < 2) {
		return;
	}
	int ReserveSize = NumPoints * 4;

	TArray<FVector> Vertices;
	Vertices.Reserve(ReserveSize);

	TArray<int32> Triangles;
	Triangles.Reserve((NumPoints - 1) * 6);

	TArray<FVector> Normals;
	Normals.Reserve(ReserveSize);

	TArray<FVector2D> UVs;
	UVs.Reserve(ReserveSize);

	TArray<FColor> VertexColors;
	VertexColors.Init(FColor::White, ReserveSize);

	TArray<FProcMeshTangent> Tangents;
	Tangents.Reserve(ReserveSize);

	for (int32 i = 0; i < NumPoints; ++i) {

		const FVector& Current = TrailPoints[i];
		FVector Direction;

		if (i == 0) {
			Direction = (TrailPoints[i + 1] - Current).GetSafeNormal();
		}
		else {
			Direction = (Current - TrailPoints[i - 1]).GetSafeNormal();
		}

		FVector Right = FVector::CrossProduct(FVector::UpVector, Direction).GetSafeNormal() * TrailHalfWidth;
		FVector UpOffset = FVector::UpVector * TrailHalfWidth;

		Vertices.Add(Current + Right); // left vertex
		Vertices.Add(Current - Right); // right vertex

		Vertices.Add(Current + UpOffset);
		Vertices.Add(Current - UpOffset);

		FVector HorizontalNormal = FVector::UpVector;
		FVector VerticalNormal = FVector::CrossProduct(Direction, HorizontalNormal).GetSafeNormal();

		Normals.Add(HorizontalNormal);
		Normals.Add(HorizontalNormal);
		Normals.Add(VerticalNormal);
		Normals.Add(VerticalNormal);

		UVs.Add(FVector2D(0.0f, i * 0.1f));
		UVs.Add(FVector2D(1.0f, i * 0.1f));
		UVs.Add(FVector2D(0.0f, i * 0.1f));
		UVs.Add(FVector2D(1.0f, i * 0.1f));

		Tangents.Add(FProcMeshTangent(Direction, false));
		Tangents.Add(FProcMeshTangent(Direction, false));
		Tangents.Add(FProcMeshTangent(HorizontalNormal, false));
		Tangents.Add(FProcMeshTangent(HorizontalNormal, false));

		if (i < NumPoints - 1) {
			int32 Index = i * 4;

			// horizontal face
			Triangles.Add(Index);
			Triangles.Add(Index + 4);
			Triangles.Add(Index + 1);

			Triangles.Add(Index + 1);
			Triangles.Add(Index + 4);
			Triangles.Add(Index + 5);

			// vertical face
			Triangles.Add(Index + 2);
			Triangles.Add(Index + 6);
			Triangles.Add(Index + 3);

			Triangles.Add(Index + 3);
			Triangles.Add(Index + 6);
			Triangles.Add(Index + 7);
		}
	}

	TrailMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, false);
}
