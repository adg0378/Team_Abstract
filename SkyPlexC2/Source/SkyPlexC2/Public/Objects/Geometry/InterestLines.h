// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include <CesiumGeoreference.h>
#include "InterestLines.generated.h"


UCLASS()
class SKYPLEXC2_API AInterestLines : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInterestLines();

	UFUNCTION(BlueprintCallable)
	// Clears all points and then destroys this Spline Actor
	void DestroyInterestLines();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SpawnInterestLines();
	void SpawnInterestLines_Implementation();

	UFUNCTION(BlueprintCallable)
	void DrawLinesViaActorLocations(TArray<FVector> ActorPoints, FString tag);

	UFUNCTION(BlueprintCallable)
	void DrawLinesViaLonLatHgts(TArray<FVector> LonLatHgts, FString tag);

	UFUNCTION(BlueprintNativeEvent)
	void SetLineColorViaTag(const FString& tag);
	void SetLineColorViaTag_Implementation(const FString& tag);

	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetLineLonLatHgts();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USplineComponent* DrawingSpline;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
