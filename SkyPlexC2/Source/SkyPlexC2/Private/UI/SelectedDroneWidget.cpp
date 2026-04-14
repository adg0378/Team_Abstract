// Copyright (c) 2025 Synetos Aerospace

#include "UI/SelectedDroneWidget.h"
#include "SPUtility.h"
#include "State/SPGameState.h"
#include "SPDroneManager.h"
#include "BasicDrone.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Internationalization/Text.h"

void USelectedDroneWidget::UpdateViewWithDroneData()
{
  ABasicDrone *SelectedDrone = USPUtility::GetSPGameState(this)->droneManagerRef->getSelectedDrone();

  if (SelectedDrone)
  {
    // Drone name
    if (DroneNameText)
    {
      DroneNameText->SetText(FText::FromString(SelectedDrone->GetName()));
    }

    // Swarm ID (int → FText is OK)
    if (SwarmIDText)
    {
      SwarmIDText->SetText(FText::AsNumber(SelectedDrone->GetSwarmID()));
    }

    // Latitude (float → string → FText)
    if (LatitudeText)
    {
      LatitudeText->SetText(
          FText::FromString(FString::SanitizeFloat(SelectedDrone->GetPosition().lat)));
    }

    // Longitude (float → string → FText)
    if (LongitudeText)
    {
      LongitudeText->SetText(
          FText::FromString(FString::SanitizeFloat(SelectedDrone->GetPosition().lon)));
    }

    // Altitude (float → string → FText)
    if (AltitudeText)
    {
      AltitudeText->SetText(
          FText::FromString(FString::SanitizeFloat(SelectedDrone->GetPosition().alt_msl)));
    }

    if (SpeedText)
    {
      SpeedText->SetText(
          FText::FromString(FString::SanitizeFloat(0.0f)));
    }

    // Battery (0–100 → 0–1)
    if (BatteryProgressBar)
    {
      BatteryProgressBar->SetPercent(SelectedDrone->GetStatus().battery_remaining / 100.0f);
    }
  }
}