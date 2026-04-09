// Copyright (c) 2025 Synetos Aerospace

#include "UI/SelectedDroneWidget.h"
#include "SPUtility.h"
#include "State/SPGameState.h"
#include "SPDroneManager.h"
#include "BasicDrone.h"

void USelectedDroneWidget::UpdateViewWithDroneData()
{
  ABasicDrone *SelectedDrone = USPUtility::GetSPGameState(this)->droneManagerRef->getSelectedDrone();

  if (SelectedDrone)
  {
    // Print out drone data
    UE_LOG(LogTemp, Log, TEXT("Selected drone: %s"), *SelectedDrone->GetName());
    UE_LOG(LogTemp, Log, TEXT("Swarm ID: %i"), SelectedDrone->GetSwarmID());
    FCCSimPosition Position = SelectedDrone->GetPosition();
    UE_LOG(LogTemp, Log, TEXT("Position: %f, %f, %f"), Position.lat, Position.lon, Position.alt_msl);
    FCCSimStatus Status = SelectedDrone->GetStatus();
    UE_LOG(LogTemp, Log, TEXT("Battery remaining: %f%%"), Status.battery_remaining);
  }
}
