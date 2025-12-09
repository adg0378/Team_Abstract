// Fill out your copyright notice in the Description page of Project Settings.


#include "SPDroneManager.h"
#include "SPLogger.h"
#include <SPUtility.h>
#include "State/PreferencesManager.h"
#include "State/Storage/SPStorageManager.h"
#include "State/Storage/MissionAssetsManager.h"
#include "State/SPGameState.h"
#include "Objects/Drones/CCSimDrone.h"
#include "GeographicUtility.h"
#include "Misc/CoreDelegates.h"
#include "BasicDrone.h"
#include <State/Missions/SPMissionManager.h>

void USPDroneManager::Setup_Implementation(bool& outSuccess) {
    BindToPreferencesUpdates = true;
    Super::Setup_Implementation(outSuccess);
    cli = NewObject<UCLIUtility>(this);
    LoadSwarms();
}

void USPDroneManager::Teardown_Implementation() {
    UCLIUtility::KillWSLProc(cli->wslProc);
}

void USPDroneManager::ApplyPreferencesUpdates_Implementation(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences) {
    if (PrevPreferences.CCSimPreferences != NewPreferences.CCSimPreferences ||
        PrevPreferences.DronePreferences != NewPreferences.DronePreferences) {

        for (const TPair<int32, FSPSwarmStruct>& Pair : Drones) {
            for (const TPair<int32, ABasicDrone*> DronePair : Pair.Value.Drones) {
                DronePair.Value->SetPreferences(NewPreferences.CCSimPreferences, NewPreferences.DronePreferences);
            }
        }
    }


    //if (PrevPreferences.CCSimPreferences != NewPreferences.CCSimPreferences) {
    //    // TODO: if starting host and port change then maybe we should update all sim drones? Or no?
    //    if (NewPreferences.CCSimPreferences.Enabled) {
    //        // TODO: add functionality to disconnect all sim drones when this is disabled
    //    }
    //}
}

FString USPDroneManager::GetSwarmName(const int32 SwarmID) const {
    const FString* Name = Swarms.Find(SwarmID);
    if (Name == nullptr) {
        return TEXT("Unknown");
    }
    return *Name;
}

ABasicDrone* USPDroneManager::GetDrone(const int32 DroneID, const int32 SwarmID) const {
    const FSPSwarmStruct* SwarmDrones = Drones.Find(SwarmID);
    if (!SwarmDrones) {
        LOG->Error(TEXT("Unable to get drone: swarm does not exist"), LOG_ORIGIN);
        return nullptr;
    }

    ABasicDrone* Drone = SwarmDrones->Drones.Find(DroneID)->Get();
    if (!Drone) {
        LOG->Error(TEXT("Unable to get drone: drone does not exist"), LOG_ORIGIN);
        return nullptr;
    }
    return Drone;
}

const TMap<int32, FSPSwarmStruct>& USPDroneManager::GetDrones() const {
    return Drones;
}

int32 USPDroneManager::GetUnassignedSwarmID() const {
    return UnassignedID;
}

const TMap<int32, FString>& USPDroneManager::GetSwarms() const {
    return Swarms;
}

TArray<const FSPSwarmStruct*> USPDroneManager::GetSwarmDrones(const TArray<int32>& SwarmIDs) const {
    TArray<const FSPSwarmStruct*> SwarmDrones;

    for (const int32& ID : SwarmIDs) {
        if (Drones.Contains(ID)) {
            SwarmDrones.Add(Drones.Find(ID));
        }
    }
    return SwarmDrones;
}

int USPDroneManager::GetNumSwarmDrones(const TArray<int32>& SwarmIDs) const {
    int total = 0;

    for (const int32& ID : SwarmIDs) {
        if (Drones.Contains(ID)) {
            const FSPSwarmStruct* SwarmDrones = Drones.Find(ID);
            total += SwarmDrones->Drones.Num();
        }
    }
    return total;
}

int32 USPDroneManager::GetNumSimDrones() const {
    return NumCCSimDrones;
}

void USPDroneManager::AddSwarm(FString Name, int32& OutSwarmID) {
    if (Name == UNASSIGNED) {
        return;
    }

    int32 SwarmID = -1;
    gameStateRef->storageManagerRef->missionAssetsManagerRef->InsertSwarm(Name, SwarmID);

    Swarms.Add(SwarmID, Name);
    Drones.Add(SwarmID);
    OutSwarmID = SwarmID;
    OnSwarmAdded.Broadcast(SwarmID);
    LOG->Info(FString::Format(*FString("Added swarm: '{0}'"), { *Name }), LOG_ORIGIN);
}

void USPDroneManager::RenameSwarm(int32 SwarmID, FString NewName) {
    if (NewName == UNASSIGNED || !Swarms.Contains(SwarmID)) {
        return;
    }
    Swarms[SwarmID] = NewName;
    gameStateRef->storageManagerRef->missionAssetsManagerRef->RenameSwarm(SwarmID, NewName);

    gameStateRef->RefreshSelected();

    OnSwarmUpdated.Broadcast(SwarmID);
}

void USPDroneManager::DeleteSwarm(int32 InSwarmID, bool DisconnectDrones, int32 DumpSwarmID) {
    if (!Swarms.Contains(InSwarmID)) {
        return;
    }
    gameStateRef->storageManagerRef->missionAssetsManagerRef->DeleteSwarm(InSwarmID);
    Swarms.Remove(InSwarmID);

    const FSPSwarmStruct* SwarmDrones = Drones.Find(InSwarmID);
    if (SwarmDrones == nullptr) {
        LOG->Warn(FString::Printf(TEXT("Found no drone references for swarm '%i'"), InSwarmID), LOG_ORIGIN);
        return;
    }

    if (DisconnectDrones) {
        for (const TPair<int32, ABasicDrone*>& Drone : SwarmDrones->Drones) {
            IInteractionBoxProvider::Execute_DestroySelf(Drone.Value);
        }
    }
    else {
        if (DumpSwarmID == -1 || !Swarms.Contains(DumpSwarmID)) {
            DumpSwarmID = UnassignedID;
        }
        FSPSwarmStruct* DumpSwarmDrones = Drones.Find(DumpSwarmID);

        for (const TPair<int32, ABasicDrone*>& Drone : SwarmDrones->Drones) {
            Drone.Value->SetSwarmID(DumpSwarmID);
            int32 ID = Drone.Value->GetID();
            DumpSwarmDrones->Drones.Add(Drone);
            // In the future if we decide to store drone details in the database
            // add a method to store their details and update them here
        }
        OnSwarmUpdated.Broadcast(DumpSwarmID);
    }
    Drones.Remove(InSwarmID);
    OnSwarmDeleted.Broadcast(InSwarmID);
}

void USPDroneManager::LoadSwarms() {
    gameStateRef->storageManagerRef->missionAssetsManagerRef->LoadSwarms(Swarms);

    TSet<int32> OutKeys;
    Swarms.GetKeys(OutKeys);

    for (int32 SwarmId : OutKeys) {
        Drones.Add(SwarmId);
    }

    TArray<FString> Values;
    Swarms.GenerateValueArray(Values);
    if (!Values.Contains(UNASSIGNED)) {
        int32 SwarmID = -1;
        gameStateRef->storageManagerRef->missionAssetsManagerRef->InsertSwarm(UNASSIGNED, SwarmID);
        Swarms.Add(SwarmID, UNASSIGNED);
        Drones.Add(SwarmID);
        UnassignedID = SwarmID;
    }
    else {
        UnassignedID = InterestsUtil::ReverseMapInterestGroup(Swarms, UNASSIGNED);
    }
}

void USPDroneManager::AddDrone(int32 SwarmID, bool IsCCSim) {
    if (SwarmID == -1 || !Swarms.Contains(SwarmID)) {
        SwarmID = UnassignedID;
    }

    FSPSwarmStruct* SwarmDrones = Drones.Find(SwarmID);
    if (!SwarmDrones) {
        LOG->Error(TEXT("Unable to spawn: swarm does not exist"), LOG_ORIGIN);
        return;
    }

    FSPPreferencesStruct Prefs = PreferencesManagerRef->GetPreferencesRef();
    if (IsCCSim) {
        if (!Prefs.CCSimPreferences.Enabled) {
            LOG->Alert(TEXT("Cannot spawn CC Sim drone: CC Simulation is not enabled. Please enable CC Simulation in preferences and try again."));
        }

        FVector MaybePos = gameStateRef->CesiumGeoreference->TransformUnrealPositionToLongitudeLatitudeHeight(FVector::ZeroVector);
        FVector SimWorldOrigin = gameStateRef->MissionManagerRef->GetWorldOriginPos();

        // convert to NM
        double SpawnDistanceFromSimOrigin = UGeographicUtility::GeodesicDistance(MaybePos, SimWorldOrigin) * 0.000539957;

        if (SpawnDistanceFromSimOrigin >= Prefs.ConfigPreferences.SimulationRadiusNM) {
            LOG->Alert(TEXT("Too far away from simulation origin to spawn a drone. Click the target to fly to the simulation origin."), true, SimWorldOrigin);
            return;
        }

        ACCSimDrone* Drone;
        gameStateRef->GenericSpawnActor<ACCSimDrone>(CCSimDroneToSpawn, FVector::ZeroVector, Drone);
        if (!Drone) {
            LOG->Alert(TEXT("Unknown error spawning CC Sim drone"));
            return;
        }

        // move the drone a little bit above ground to avoid immediate dissapearing issues
        FVector LonLatHeight;
        Drone->GetLongitudeLatitudeHeight(LonLatHeight);
        LonLatHeight.Z += 100;
        Drone->MoveToLongitudeLatitudeHeight(LonLatHeight);

        // add validation that given port is not occupied before finding another port
        int32 ID = CCSimPort;

        if (!StartedDDSBridge) {
            StartedDDSBridge = true;
            cli->StartXRCEAgent(cli->wslProc);
        }

        cli->AddDroneProcesses(Drone->PX4Processes, NumCCSimDrones, ID, SimWorldOrigin, LonLatHeight, [this, Drone, SwarmID, ID]() {
            UE_LOG(LogTemp, Log, TEXT("[DEBUG] Entered OnReady()"));
            Drone->Setup(CCSimHost, ID, LOG);
            Drone->SetSwarmID(SwarmID);
            Drone->SetID(ID);
            Drones[SwarmID].Drones.Add(ID, Drone);
            OnDroneAdded.Broadcast(ID, SwarmID);
            });

        Drone->SetPreferences(Prefs.CCSimPreferences, Prefs.DronePreferences);
        SwarmDrones->Drones.Add(ID, Drone);
        CCSimPort++;
        NumCCSimDrones++;
        //LOG->Alert(FString::Printf(TEXT("drone spawned with id %i and swarn id %i"), ID, SwarmID));
    }
    // TODO: Implement other drone capabilities
}

void USPDroneManager::ReassignDrone(int32 DroneID, int32 CurrSwarmID, int32 NewSwarmID) {
    FSPSwarmStruct* NewSwarmDrones = Drones.Find(NewSwarmID);
    if (!NewSwarmDrones) {
        LOG->Error(TEXT("Unable to assign drone: swarm does not exist"), LOG_ORIGIN);
        return;
    }

    FSPSwarmStruct* OldSwarmDrones = Drones.Find(CurrSwarmID);
    ABasicDrone* Drone = OldSwarmDrones->Drones.Find(DroneID)->Get();

    if (!Drone) {
        LOG->Error(TEXT("Unable to assign drone: drone does not exist"), LOG_ORIGIN);
        return;
    }

    Drone->SetSwarmID(NewSwarmID);

    // TODO: If we ever store drone details in the database, update them here

    gameStateRef->RefreshSelected();

    OldSwarmDrones->Drones.Remove(DroneID);
    NewSwarmDrones->Drones.Add(DroneID, Drone);

    OnSwarmUpdated.Broadcast(CurrSwarmID);
    OnSwarmUpdated.Broadcast(NewSwarmID);
}

void USPDroneManager::ReassignDrones(UPARAM(ref) const TMap<int32, int32>& IDToSwarmMap, int32 NewSwarmID) {
    FSPSwarmStruct* NewSwarmDrones = Drones.Find(NewSwarmID);
    if (!NewSwarmDrones) {
        LOG->Error(TEXT("Unable to assign drones: swarm does not exist"), LOG_ORIGIN);
        return;
    }

    TSet<int32> SwarmsToUpdate = { NewSwarmID };

    for (const TPair<int32, int32>& IDToSwarmPair : IDToSwarmMap) {
        FSPSwarmStruct* OldSwarmDrones = Drones.Find(IDToSwarmPair.Value);
        ABasicDrone* Drone = OldSwarmDrones->Drones.Find(IDToSwarmPair.Key)->Get();

        if (!Drone) {
            LOG->Error(TEXT("Unable to assign drone: drone does not exist"), LOG_ORIGIN);
            continue;
        }

        Drone->SetSwarmID(NewSwarmID);
        OldSwarmDrones->Drones.Remove(IDToSwarmPair.Key);
        NewSwarmDrones->Drones.Add(IDToSwarmPair.Key, Drone);
        SwarmsToUpdate.Add(IDToSwarmPair.Value);
    }

    gameStateRef->RefreshSelected();

    for (int32& ID : SwarmsToUpdate) {
        OnSwarmUpdated.Broadcast(ID);
    }
}

void USPDroneManager::RenameDrone(int32 DroneID, int32 SwarmID, FString NewName) {
    FSPSwarmStruct* SwarmDrones = Drones.Find(SwarmID);
    if (!SwarmDrones) {
        LOG->Error(TEXT("Unable to rename drone: swarm does not exist"), LOG_ORIGIN);
        return;
    }
    ABasicDrone* Drone = SwarmDrones->Drones.Find(DroneID)->Get();
    if (!Drone) {
        LOG->Error(TEXT("Unable to rename drone: drone does not exist"), LOG_ORIGIN);
        return;
    }

    Drone->SetName(NewName);
    gameStateRef->RefreshSelected();

    OnDroneUpdated.Broadcast(DroneID, SwarmID);
}

void USPDroneManager::RemoveDrone(int32 DroneID, int32 SwarmID) {
    FSPSwarmStruct* SwarmDrones = Drones.Find(SwarmID);
    if (!SwarmDrones) {
        LOG->Error(TEXT("Unable to remove drone: swarm does not exist"), LOG_ORIGIN);
        return;
    }

    ABasicDrone* Drone = SwarmDrones->Drones.Find(DroneID)->Get();
    if (!Drone) {
        LOG->Error(TEXT("Unable to remove drone: drone does not exist"), LOG_ORIGIN);
        return;
    }

    SwarmDrones->Drones.Remove(DroneID);
    NumCCSimDrones--;
    OnDroneDeleted.Broadcast(DroneID, SwarmID);
}

void USPDroneManager::CullRelatedObjects(float MaximumDrawDistance, const FVector& OriginLocationUE, bool FromFlyTo) {
    for (const TPair<int32, FSPSwarmStruct>& Pair : Drones) {
        for (const TPair<int32, ABasicDrone*>& DronePair : Pair.Value.Drones) {
            FVector LocationToCheck = DronePair.Value->GetActorLocation();

            FVector DistanceVector = OriginLocationUE - LocationToCheck;
            bool IsCulled = DistanceVector.Size() > MaximumDrawDistance;
            IInteractionBoxProvider::Execute_ToggleCull(DronePair.Value, IsCulled, FromFlyTo);
        }
    }
}

int32 USPDroneManager::GetCurrPortNum() {
    return CCSimPort;
}