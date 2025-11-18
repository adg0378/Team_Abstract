// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Drones/SPCCSimDrone.h"
#include "Objects/Drones/SPCCSimMessenger.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Core/SPLogger.h"
#include "Core/State/SPGameState.h"
#include "Util/SPGeoUtility.h"
#include "Core/SPPreferences.h"
#include "Util/SPAssetUtility.h"
#include "CesiumGlobeAnchorComponent.h"
#include "Core/State/SPMissionManager.h"

void ASPCCSimDrone::Setup(FString InHost, int32 InPort, USPLogger* InLog) {
    LOG = InLog;
    SetName(FString::Printf(TEXT("CCSim_%i"), InPort));
    InitializeWebsocket(InHost, InPort);
}

void ASPCCSimDrone::SetPreferences(const FCCSimPreferencesStruct& InPrefs, const FDronePreferencesStruct& InDronePrefs) {
    Super::SetPreferences(InPrefs, InDronePrefs);
    ShowAltitudeReferenceLineTraces = InPrefs.ShowAltitudeReferenceLineTraces;
    AltitudeRefreshDistance = InPrefs.AltitudeRefreshDistanceM;
}

void ASPCCSimDrone::DestroySelf_Implementation() {
    CloseWebsocket();
    Super::DestroySelf_Implementation();
}
void ASPCCSimDrone::BeginDestroy() {
    CloseWebsocket();
    FPlatformProcess::TerminateProc(this->PX4Processes.px4Handle);
    FPlatformProcess::TerminateProc(this->PX4Processes.ros2Handle);
    Super::BeginDestroy();
}

void ASPCCSimDrone::InitializeWebsocket(FString InHost, int32 InPort) {
    // The WebSockets module is known to sometimes not load in packaged builds
    if (!FModuleManager::Get().IsModuleLoaded("WebSockets")) {
        FModuleManager::Get().LoadModule("WebSockets");
    }

    Host = InHost;
    Port = InPort;

    ConnectWebsocket();
}

void ASPCCSimDrone::ConnectWebsocket() {
    if (Websocket.Get() && Websocket->IsConnected()) {
        return;
    }

    FString Url = FString::Printf(TEXT("ws://%s:%i"), *Host, Port);
    Websocket = FWebSocketsModule::Get().CreateWebSocket(Url);

    Websocket->OnConnectionError().AddLambda([this](const FString& Error) {
        Connected = false;

        if (!IsEngineExitRequested()) {
            FString Url = FString::Printf(TEXT("ws://%s:%i"), *Host, Port);
            FString Message = FString::Printf(TEXT("Error connecting to CC Sim web socket '%s': %s"), *Url, *Error);
            LOG->Error(Message, Name, true);
        }
        });

    Websocket->OnConnected().AddLambda([this]() {
        FString Url = FString::Printf(TEXT("ws://%s:%i"), *Host, Port);
        Connected = true;
        LOG->Info(FString::Printf(TEXT("Connected to %s"), *Url), Name);
        });

    Websocket->OnClosed().AddLambda([this](int32 StatusCode, const FString& Reason, bool WasClean) {
        /*if (!IsEngineExitRequested()) {
            FString Url = FString::Printf(TEXT("ws://%s:%i"), *HostCCSim, PortCCSim);
            if (WasClean) {
                LOG->Info(FString::Printf(TEXT("Gracefully closed CC Sim web socket '%s': %i - %s"), *Url, StatusCode, *Reason));
            }
            else {
                FString Msg = FString::Printf(TEXT("Suddenly closed CC Sim web socket '%s': %i - %s"), *Url, StatusCode, *Reason);
                LOG->Error(Msg);
                LOG->Alert(Msg);
            }
        }*/
        });

    Websocket->OnMessage().AddLambda([this](const FString& Message) {
        HandleMessage(Message);
        });

    Websocket->Connect();
}

void ASPCCSimDrone::CloseWebsocket() {
    if (Websocket.IsValid() && Websocket->IsConnected()) {
        Websocket->Close();
        Connected = false;
    }
}

void ASPCCSimDrone::HandleMessage(const FString& Message) {
    // UE_LOG(LogTemp, Log, TEXT("Msg: %s"), *Message)
    ECCSimRecieveChannel Channel;
    FCCSimDataVariant Data = USPCCSimMessenger::ParseCCSimMessage(Message, &Channel);
    switch (Channel) {
    case ECCSimRecieveChannel::POSITION:
        if (Data.IsType<FCCSimPosition>()) {
            SetPosition(Data.Get<FCCSimPosition>());
        }
        break;
    case ECCSimRecieveChannel::STATUS:
        if (Data.IsType<FCCSimStatus>()) {
            SetStatus(Data.Get<FCCSimStatus>());
        }
        break;
    case ECCSimRecieveChannel::CMD_ACK:
        IsAwaitingCommand = false;
        if (Data.IsType<FCCSimCommandAck>()) {
            FCCSimCommandAck& Ack = Data.Get<FCCSimCommandAck>();
            if (Ack.result != 0) {
                ASPGameState* GameState = ASPGameState::GetSPGameState(this);
                FString Msg = FString::Printf(TEXT("Command execution error: %s (%i)"), *Ack.result_name, Ack.result);
                GameState->Logger->Error(Msg, Name, true);
            }
        }
        break;
    case ECCSimRecieveChannel::MISSION_ACK:
        if (Data.IsType<FCCSimMissionAck>()) {
            FCCSimMissionAck& MissionAck = Data.Get<FCCSimMissionAck>();
            if (MissionAck.result != EMissionUploadAckResult::SUCCESS) {
                ASPGameState* GameState = ASPGameState::GetSPGameState(this);
                FString Msg = FString::Printf(TEXT("Failed to upload mission: %s (%i)"), *MissionAck.result_name, MissionAck.result);
                GameState->Logger->Error(Msg, Name, true);
            }
        }
        break;
    case ECCSimRecieveChannel::MISSION_FINISHED:
        if (Data.IsType<FCCSimMissionFinished>()) {
            ASPGameState* GameState = ASPGameState::GetSPGameState(this);
            GameState->MissionManager->CompleteMission(SwarmID);
        }
    default:
        uint8 Val = static_cast<uint8>(Channel);
        UE_LOG(LogTemp, Error, TEXT("Channel unaccounted for: %d"), static_cast<int32>(Val))
    }
}

void ASPCCSimDrone::SetStatus(const FCCSimStatus& InStatus) {
    Super::SetStatus(InStatus);

    // Currently check sample altitude here to reduce the time of check
    // This may need to be moved somewhere else if the default SetStatus websocket
    // refresh time is ever increased in the future.

    StatusUpdatesSinceLastSample++;
    if ((Position.lon - PosAtLastAltitudeSample.X) * SPGeoMath::METERS_PER_LON >= AltitudeRefreshDistance ||
        (Position.lat - PosAtLastAltitudeSample.Y) * SPGeoMath::METERS_PER_LAT >= AltitudeRefreshDistance ||
        StatusUpdatesSinceLastSample > 5) {
        SampleAltitude();
    }
}

void ASPCCSimDrone::SetPosition(const FCCSimPosition& PositionData) {
    Position = PositionData;

    // Effectively convert msl into agl for simulation ease and efficiency
    GlobeAnchorComponent->MoveToLongitudeLatitudeHeight(FVector(Position.lon, Position.lat, TilesetHeightM + Position.alt_msl));

    // Subtract 90 from yaw because we are setting EastSouthUp. Subtract -90 to set based on North
    this->GlobeAnchorComponent->SetEastSouthUpRotation(FQuat::MakeFromEuler(FVector(Position.roll, Position.pitch, Position.yaw - 90)));
}

void ASPCCSimDrone::SampleAltitude() {
    const int TRACE_DISTANCE = 100000;
    StatusUpdatesSinceLastSample = 0;

    UWorld* World = GetWorld();
    ECollisionChannel LandscapeChannel = USPAssetUtility::LandscapeTraceChannel;

    PosAtLastAltitudeSample = GlobeAnchorComponent->GetLongitudeLatitudeHeight();

    const FVector& Start = GetActorLocation();
    const FVector& Up = GetActorUpVector();

    FHitResult HitResult;


    if (World->LineTraceSingleByChannel(HitResult, Start, Start - (Up * TRACE_DISTANCE), LandscapeChannel)) {
        TilesetHeightM = PosAtLastAltitudeSample.Z - (HitResult.Distance / 100);

        if (ShowAltitudeReferenceLineTraces) {
            DrawDebugLine(World, Start, Start - (Up * TRACE_DISTANCE), FColor::Magenta, false, 5.0f, 0U, 3.0F);
        }
        return;
    }


    if (World->LineTraceSingleByChannel(HitResult, Start + (Up * TRACE_DISTANCE), Start, LandscapeChannel)) {
        TilesetHeightM = PosAtLastAltitudeSample.Z + ((TRACE_DISTANCE - HitResult.Distance) / 100);

        if (ShowAltitudeReferenceLineTraces) {
            DrawDebugLine(World, Start + (Up * TRACE_DISTANCE), Start, FColor::Magenta, false, 5.0f, 0U, 3.0F);
        }
    }
}

void ASPCCSimDrone::SendCommand(FString Message) {
    if (!IsAwaitingCommand) {
        IsAwaitingCommand = true;
        Websocket->Send(Message);
    }

}

void ASPCCSimDrone::UploadMission(const FString& Mission) {
    Websocket->Send(Mission);
}

void ASPCCSimDrone::Arm() {
    TSharedPtr<FJsonObject> ParamsObject = MakeShared<FJsonObject>();

    SendCommand(USPCCSimMessenger::MakeCCSimCommandMessage(ECCSimCommand::ARM, ParamsObject));
}

void ASPCCSimDrone::Disarm() {
    TSharedPtr<FJsonObject> ParamsObject = MakeShared<FJsonObject>();

    SendCommand(USPCCSimMessenger::MakeCCSimCommandMessage(ECCSimCommand::DISARM, ParamsObject));
}


void ASPCCSimDrone::Takeoff(float Lat, float Lon, float Altitude) {
    TSharedPtr<FJsonObject> ParamsObject = MakeShared<FJsonObject>();
    ParamsObject->SetNumberField(TEXT("lat"), Lat);
    ParamsObject->SetNumberField(TEXT("lon"), Lon);
    ParamsObject->SetNumberField(TEXT("altitude"), Altitude);

    SendCommand(USPCCSimMessenger::MakeCCSimCommandMessage(ECCSimCommand::TAKEOFF, ParamsObject));
}


void ASPCCSimDrone::ArmTakeoff(float Lat, float Lon, float Altitude) {
    TSharedPtr<FJsonObject> ParamsObject = MakeShared<FJsonObject>();
    ParamsObject->SetNumberField(TEXT("lat"), Lat);
    ParamsObject->SetNumberField(TEXT("lon"), Lon);
    ParamsObject->SetNumberField(TEXT("altitude"), Altitude);

    SendCommand(USPCCSimMessenger::MakeCCSimCommandMessage(ECCSimCommand::ARM_TAKEOFF, ParamsObject));
}


void ASPCCSimDrone::Land() {
    TSharedPtr<FJsonObject> ParamsObject = MakeShared<FJsonObject>();

    SendCommand(USPCCSimMessenger::MakeCCSimCommandMessage(ECCSimCommand::LAND, ParamsObject));
}


void ASPCCSimDrone::ChangeAltitude(float Altitude) {
    TSharedPtr<FJsonObject> ParamsObject = MakeShared<FJsonObject>();
    ParamsObject->SetNumberField(TEXT("altitude"), Altitude);

    SendCommand(USPCCSimMessenger::MakeCCSimCommandMessage(ECCSimCommand::CHANGE_ALTITUDE, ParamsObject));
}


void ASPCCSimDrone::Return() {
    TSharedPtr<FJsonObject> ParamsObject = MakeShared<FJsonObject>();

    SendCommand(USPCCSimMessenger::MakeCCSimCommandMessage(ECCSimCommand::RETURN, ParamsObject));
}


void ASPCCSimDrone::Orbit(float Radius, float Velocity, float Lat, float Lon, float Altitude) {
    TSharedPtr<FJsonObject> ParamsObject = MakeShared<FJsonObject>();
    ParamsObject->SetNumberField(TEXT("radius"), Radius);
    ParamsObject->SetNumberField(TEXT("velocity"), Velocity);
    ParamsObject->SetNumberField(TEXT("lat"), Lat);
    ParamsObject->SetNumberField(TEXT("lon"), Lon);
    ParamsObject->SetNumberField(TEXT("altitude"), Altitude);

    SendCommand(USPCCSimMessenger::MakeCCSimCommandMessage(ECCSimCommand::ORBIT, ParamsObject));
}


void ASPCCSimDrone::Reposition(float Velocity, float Lon, float Lat, float Altitude) {
    TSharedPtr<FJsonObject> ParamsObject = MakeShared<FJsonObject>();
    ParamsObject->SetNumberField(TEXT("velocity"), Velocity);
    ParamsObject->SetNumberField(TEXT("lat"), Lat);
    ParamsObject->SetNumberField(TEXT("lon"), Lon);
    ParamsObject->SetNumberField(TEXT("altitude"), Altitude);

    SendCommand(USPCCSimMessenger::MakeCCSimCommandMessage(ECCSimCommand::REPOSITION, ParamsObject));
}


void ASPCCSimDrone::OffboardOn() {
    TSharedPtr<FJsonObject> ParamsObject = MakeShared<FJsonObject>();

    SendCommand(USPCCSimMessenger::MakeCCSimCommandMessage(ECCSimCommand::OFFBOARD_ON, ParamsObject));
}


void ASPCCSimDrone::OffboardOff() {
    TSharedPtr<FJsonObject> ParamsObject = MakeShared<FJsonObject>();

    SendCommand(USPCCSimMessenger::MakeCCSimCommandMessage(ECCSimCommand::OFFBOARD_OFF, ParamsObject));
}


void ASPCCSimDrone::SetPoint(FVector Point) {
    TSharedPtr<FJsonObject> ParamsObject = MakeShared<FJsonObject>();

    TArray<TSharedPtr<FJsonValue>> PointArray;
    PointArray.Add(MakeShared<FJsonValueNumber>(Point.X));
    PointArray.Add(MakeShared<FJsonValueNumber>(Point.Y));
    PointArray.Add(MakeShared<FJsonValueNumber>(Point.Z));

    ParamsObject->SetArrayField(TEXT("point"), PointArray);

    SendCommand(USPCCSimMessenger::MakeCCSimCommandMessage(ECCSimCommand::SET_POINT, ParamsObject));
}


void ASPCCSimDrone::SetWaypoint(float HoldTime, float AcceptanceRadius, float PassRadius, float Yaw, float Lat, float Lon, float Altitude) {
    TSharedPtr<FJsonObject> ParamsObject = MakeShared<FJsonObject>();
    ParamsObject->SetNumberField(TEXT("hold_time"), HoldTime);
    ParamsObject->SetNumberField(TEXT("acceptance_radius"), AcceptanceRadius);
    ParamsObject->SetNumberField(TEXT("pass_radius"), PassRadius);
    ParamsObject->SetNumberField(TEXT("yaw"), Yaw);
    ParamsObject->SetNumberField(TEXT("lat"), Lat);
    ParamsObject->SetNumberField(TEXT("lon"), Lon);
    ParamsObject->SetNumberField(TEXT("altitude"), Altitude);

    SendCommand(USPCCSimMessenger::MakeCCSimCommandMessage(ECCSimCommand::SET_WAYPOINT, ParamsObject));
}


void ASPCCSimDrone::Formation() {
    TSharedPtr<FJsonObject> ParamsObject = MakeShared<FJsonObject>();

    SendCommand(USPCCSimMessenger::MakeCCSimCommandMessage(ECCSimCommand::FORMATION, ParamsObject));
}