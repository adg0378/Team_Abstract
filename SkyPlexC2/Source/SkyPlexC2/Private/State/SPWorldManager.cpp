// Copyright (c) 2025 Synetos Aerospace


#include "State/SPWorldManager.h"
#include "State/PreferencesManager.h"
#include "State/Missions/SPMissionManager.h"
#include "SPDroneManager.h"
#include "State/Obstacles/SPObstacleManager.h"
#include <CesiumSunSky.h>

#undef GetObject

void USPWorldManager::Setup_Implementation(bool& outSuccess) {
	BindToPreferencesUpdates = true;
	Super::Setup_Implementation(outSuccess);

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACesiumSunSky::StaticClass(), FoundActors);
	const FWorldPreferencesStruct& WorldPrefs = PreferencesManagerRef->GetPreferencesRef().WorldPreferences;

	if (FoundActors.Num() > 0)
	{
		SunSky = Cast<ACesiumSunSky>(FoundActors[0]);
		SunSky->SolarTime = WorldPrefs.SolarTime;
		SunSky->Year = WorldPrefs.Year;
		SunSky->Month = WorldPrefs.Month > 12 ? 9 : WorldPrefs.Month;
		SunSky->Day = WorldPrefs.Day;
		SunSky->UpdateSun();
	}
	else {
		LOG->Error(TEXT("Error fetching sun and sky"));
	}
	gameStateRef->CesiumTileset->SetMaximumScreenSpaceError(WorldPrefs.TilesetLevelOfDetail);
	gameStateRef->CesiumTileset->ShowCreditsOnScreen = false;
}

void USPWorldManager::ApplyPreferencesUpdates_Implementation(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences) {
	if (PrevPreferences.WorldPreferences != NewPreferences.WorldPreferences) {
		if (SunSky) {
			SunSky->SolarTime = NewPreferences.WorldPreferences.SolarTime;
			SunSky->Year = NewPreferences.WorldPreferences.Year;
			SunSky->Month = NewPreferences.WorldPreferences.Month > 12 ? 9 : NewPreferences.WorldPreferences.Month;
			SunSky->Day = NewPreferences.WorldPreferences.Day;
			SunSky->UpdateSun();
		}
		gameStateRef->CesiumTileset->SetMaximumScreenSpaceError(NewPreferences.WorldPreferences.TilesetLevelOfDetail);
	}
}

void USPWorldManager::UpdateTimezone(float Longitude) {
	if (SunSky) {
		SunSky->TimeZone = Longitude / 15.0f;
		UE_LOG(LogTemp, Warning, TEXT("Timezone: %f, Lon: %f"), SunSky->TimeZone, Longitude)
		SunSky->UpdateSun();
	}
}

void USPWorldManager::SetActiveCamera(const TScriptInterface<USPCameraInterface>& InActiveCamera) {
	UObject* InterfaceObj = InActiveCamera.GetObject();
	if (InterfaceObj && InterfaceObj->GetClass()->ImplementsInterface(USPCameraInterface::StaticClass())) {
		UE_LOG(LogTemp, Error, TEXT("Active Camera Set!"))
		ActiveCamera = InActiveCamera;
	}
	else {
		LOG->Error(TEXT("Provided camera does not implement USPCameraInterface"), TEXT("Set active camera"), true);
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

const float USPWorldManager::GetDevicePitch() const {
	float DevicePitch;
	ISPCameraInterface::Execute_GetDevicePitch(ActiveCamera.GetObject(), DevicePitch);
	return DevicePitch;
}

const float USPWorldManager::GetDeviceRoll() const {
	float DeviceRoll;
	ISPCameraInterface::Execute_GetDeviceRoll(ActiveCamera.GetObject(), DeviceRoll);
	return DeviceRoll;
}

const float USPWorldManager::GetDeviceYaw() const {
	float DeviceYaw;
	ISPCameraInterface::Execute_GetDeviceYaw(ActiveCamera.GetObject(), DeviceYaw);
	return DeviceYaw;
}

const FString USPWorldManager::GetDeviceName() const {
	FString DeviceName;
	ISPCameraInterface::Execute_GetDeviceName(ActiveCamera.GetObject(), DeviceName);
	return DeviceName;
}

void USPWorldManager::TimerCallCullObjects() {
	CullObjects();
}

void USPWorldManager::CullObjects(FVector OriginPosUE, bool FromFlyTo) {
	if (OriginPosUE == FVector::ZeroVector) {
		OriginPosUE = GetCameraLocation();
	}
	gameStateRef->MissionManagerRef->CullRelatedObjects(MaximumDrawDistance, OriginPosUE, FromFlyTo);
	gameStateRef->obstacleManagerRef->CullRelatedObjects(MaximumDrawDistance, OriginPosUE, FromFlyTo);
	gameStateRef->droneManagerRef->CullRelatedObjects(MaximumDrawDistance, OriginPosUE, FromFlyTo);
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