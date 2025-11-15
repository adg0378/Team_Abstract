// Copyright (c) 2025 Synetos Aerospace

#define NOMINMAX
#include "Core/State/SPWorldManager.h"
#include "CesiumSunSky.h"
#include "Kismet/GameplayStatics.h"
#include "Core/State/SPGameState.h"
#include "Core/State/SPMissionManager.h"
#include "Core/State/SPDroneManager.h"
#include "Core/State/SPObstacleManager.h"
#include "Cesium3DTileset.h"

// Must undefine GetObject macro if using camera interface GetObject
#undef GetObject

void USPWorldManager::Setup_Implementation() {
	BindToPreferencesUpdates = true;
	Super::Setup_Implementation();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACesiumSunSky::StaticClass(), FoundActors);
	const FWorldPreferencesStruct& WorldPrefs = PreferencesRef->GetPreferencesRef().WorldPreferences;

	if (FoundActors.Num() > 0)
	{
		SunSky = Cast<ACesiumSunSky>(FoundActors[0]);
		SunSky->SolarTime = WorldPrefs.SolarTime;
		SunSky->Year = WorldPrefs.Year;
		SunSky->Month = WorldPrefs.Month;
		SunSky->Day = WorldPrefs.Day;
		SunSky->UpdateSun();
	}
	else {
		LOG->Error(TEXT("Error fetching sun and sky"));
	}
	GameStateRef->CesiumTileset->SetMaximumScreenSpaceError(WorldPrefs.TilesetLevelOfDetail);
	GameStateRef->CesiumTileset->ShowCreditsOnScreen = false;
}

void USPWorldManager::ApplyPreferencesUpdates_Implementation(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences) {
	if (PrevPreferences.WorldPreferences != NewPreferences.WorldPreferences) {
		if (SunSky) {
			SunSky->SolarTime = NewPreferences.WorldPreferences.SolarTime;
			SunSky->Year = NewPreferences.WorldPreferences.Year;
			SunSky->Month = NewPreferences.WorldPreferences.Month;
			SunSky->Day = NewPreferences.WorldPreferences.Day;
			SunSky->UpdateSun();
		}
		GameStateRef->CesiumTileset->SetMaximumScreenSpaceError(NewPreferences.WorldPreferences.TilesetLevelOfDetail);
	}
}

void USPWorldManager::UpdateTimezone(float Longitude) {
	if (SunSky) {
		SunSky->TimeZone = Longitude / 15.0f;
		SunSky->UpdateSun();
	}
}

void USPWorldManager::SetActiveCamera(const TScriptInterface<USPCameraInterface>& InActiveCamera) {
	UObject* InterfaceObj = InActiveCamera.GetObject();
	if (InterfaceObj && InterfaceObj->GetClass()->ImplementsInterface(USPCameraInterface::StaticClass())) {
		ActiveCamera = InActiveCamera;
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Provided camera does not implement USPCameraInterface"));
	}
}

const float USPWorldManager::GetZoomPercentage() const {
	float ZoomPercentage;
	ISPCameraInterface::Execute_GetZoomPercentage(ActiveCamera.GetObject(), ZoomPercentage);
	return ZoomPercentage;
}


const FVector USPWorldManager::GetCameraLocation() const {
	FVector CameraLocation;
	ISPCameraInterface::Execute_GetCameraLocation(ActiveCamera.GetObject(), CameraLocation);
	return CameraLocation;
}

const FRotator USPWorldManager::GetCameraRotation() const {
	FRotator CameraRotation;
	ISPCameraInterface::Execute_GetCameraRotation(ActiveCamera.GetObject(), CameraRotation);
	return CameraRotation;
}

void USPWorldManager::SetCameraTypeFromIndex(uint8 Index) {
	ISPCameraInterface::Execute_SetCameraTypeFromIndex(ActiveCamera.GetObject(), Index);
}

void USPWorldManager::TimerCallCullObjects() {
	CullObjects();
}

void USPWorldManager::CullObjects(FVector OriginPosUE) {
	if (OriginPosUE == FVector::ZeroVector) {
		OriginPosUE = GetCameraLocation();
	}
	GameStateRef->MissionManager->CullRelatedObjects(MaximumDrawDistance, OriginPosUE);
	GameStateRef->ObstacleManager->CullRelatedObjects(MaximumDrawDistance, OriginPosUE);
	GameStateRef->DroneManager->CullRelatedObjects(MaximumDrawDistance, OriginPosUE);
}

void USPWorldManager::StartObjectDistanceCullTimer() {
	FTimerManagerTimerParameters TimerParams;
	TimerParams.bLoop = TimerParams.bMaxOncePerFrame = true;
	GetWorld()->GetTimerManager().SetTimer(ObjectDistanceCullTimerHandle, this, &USPWorldManager::TimerCallCullObjects, ObjectDistanceCullTime, TimerParams);
}

void USPWorldManager::StopObjectDistanceCullTimer() {
	GetWorld()->GetTimerManager().ClearTimer(ObjectDistanceCullTimerHandle);
}

void USPWorldManager::PostSetup() {
	StartObjectDistanceCullTimer();
}

void USPWorldManager::PreTeardown() {
	StopObjectDistanceCullTimer();
}