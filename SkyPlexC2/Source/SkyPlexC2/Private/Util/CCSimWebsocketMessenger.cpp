// Copyright (c) 2025 Synetos Aerospace


#include "Util/CCSimWebsocketMessenger.h"
#include "Objects/Drones/CCSimDrone.h"
#include "JsonObjectConverter.h"

FCCSimDataVariant UCCSimWebsocketMessenger::ParseCCSimMessage(const FString& Message, ECCSimRecieveChannel* OutChannel) {
    TSharedPtr<FJsonObject> JsonResponse;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

    FCCSimDataVariant CCSimVariant;
    FCCSimDataError Error = FCCSimDataError{};
    CCSimVariant.Set<FCCSimDataError>(Error);

    if (FJsonSerializer::Deserialize(Reader, JsonResponse) && JsonResponse.IsValid()) {
        ECCSimRecieveChannel Channel = static_cast<ECCSimRecieveChannel>(JsonResponse->GetIntegerField(TEXT("Channel")));
        const TSharedPtr<FJsonObject> DataObj = JsonResponse->GetObjectField(TEXT("Data"));

        switch (Channel) {
            case ECCSimRecieveChannel::POSITION: {
                FCCSimPosition ParsedData;
                if (FJsonObjectConverter::JsonObjectToUStruct(DataObj.ToSharedRef(), FCCSimPosition::StaticStruct(), &ParsedData)) {
                    if (OutChannel != nullptr) {
                        *OutChannel = ECCSimRecieveChannel::POSITION;
                    }
                    CCSimVariant.Set<FCCSimPosition>(ParsedData);
                }
                break;
            }
            case ECCSimRecieveChannel::STATUS: {
                FCCSimStatus ParsedData;
                if (FJsonObjectConverter::JsonObjectToUStruct(DataObj.ToSharedRef(), FCCSimStatus::StaticStruct(), &ParsedData)) {
                    if (OutChannel != nullptr) {
                        *OutChannel = ECCSimRecieveChannel::STATUS;
                    }
                    CCSimVariant.Set<FCCSimStatus>(ParsedData);
                }
                break;
            }
            case ECCSimRecieveChannel::CMD_ACK: {
                FCCSimCommandAck ParsedData;
                if (FJsonObjectConverter::JsonObjectToUStruct(DataObj.ToSharedRef(), FCCSimCommandAck::StaticStruct(), &ParsedData)) {
                    if (OutChannel != nullptr) {
                        *OutChannel = ECCSimRecieveChannel::CMD_ACK;
                    }
                    CCSimVariant.Set<FCCSimCommandAck>(ParsedData);
                }
                break;
            }
            case ECCSimRecieveChannel::MISSION_ACK: {
                FCCSimMissionAck ParsedData;
                if (FJsonObjectConverter::JsonObjectToUStruct(DataObj.ToSharedRef(), FCCSimMissionAck::StaticStruct(), &ParsedData)) {
                    if (OutChannel != nullptr) {
                        *OutChannel = ECCSimRecieveChannel::MISSION_ACK;
                    }
                    CCSimVariant.Set<FCCSimMissionAck>(ParsedData);
                }
                break;
            }
            case ECCSimRecieveChannel::MISSION_FINISHED: {
                FCCSimMissionFinished ParsedData;
                if (FJsonObjectConverter::JsonObjectToUStruct(DataObj.ToSharedRef(), FCCSimMissionFinished::StaticStruct(), &ParsedData)) {
                    if (OutChannel != nullptr) {
                        *OutChannel = ECCSimRecieveChannel::MISSION_FINISHED;
                    }
                    CCSimVariant.Set<FCCSimMissionFinished>(ParsedData);
                }
                break;
            }
            default: {
                FCCSimDataUnsupported ParsedData{};
                if (OutChannel != nullptr) {
                    *OutChannel = ECCSimRecieveChannel::Unsupported;
                }
                CCSimVariant.Set<FCCSimDataUnsupported>(ParsedData);
                break;
            }
        }

        if (CCSimVariant.IsType<FCCSimDataError>() && OutChannel != nullptr) {
            *OutChannel = ECCSimRecieveChannel::Error;
        }
    }
    return CCSimVariant;
}

FString UCCSimWebsocketMessenger::MakeCCSimCommandMessage(ECCSimCommand Command, TSharedPtr<FJsonObject> Params) {
    TSharedPtr<FJsonObject> DataObject = MakeShared<FJsonObject>();

    DataObject->SetNumberField(TEXT("command"), static_cast<uint8>(Command));
    DataObject->SetObjectField(TEXT("params"), Params);

    return MakeResponseMessage(ECCSimRespondChannel::COMMAND, DataObject);
}

FString UCCSimWebsocketMessenger::MakeResponseMessage(ECCSimRespondChannel Channel, TSharedPtr<FJsonObject> Data) {
    TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
    RootObject->SetNumberField(TEXT("channel"), static_cast<uint8>(Channel));
    RootObject->SetObjectField(TEXT("data"), Data);

    FString MsgString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&MsgString);
    if (FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer)) {
        return MsgString;
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to serialize json websocket response message"))
            return TEXT("");
    }
}

FString UCCSimWebsocketMessenger::MakeMissionUploadMessage(EMissionUploadAction Action, TSharedPtr<FJsonObject> Plan) {
    TSharedPtr<FJsonObject> DataObject = MakeShared<FJsonObject>();
    DataObject->SetObjectField(TEXT("plan"), Plan);
    DataObject->SetNumberField(TEXT("action"), static_cast<uint8>(Action));

    return MakeResponseMessage(ECCSimRespondChannel::MISSION, DataObject);
}

FCommandWidgetSchema UCCSimWebsocketMessenger::GetCommandWidgetSchema(ECCSimCommand Command) {
    switch (Command) {
        case ECCSimCommand::TAKEOFF:
        case ECCSimCommand::ARM_TAKEOFF:
            return FCommandWidgetSchema(
                FCommandWidgetParamSchema(ECommandWidgetParamType::LATITUDE, -999.999, TEXT("Latitude")),
                FCommandWidgetParamSchema(ECommandWidgetParamType::LONGITUDE, -999.999, TEXT("Longitude")),
                FCommandWidgetParamSchema(ECommandWidgetParamType::ALTITUDE, 410.5, TEXT("Altitude"), TEXT("m agl"))
            );
        case ECCSimCommand::CHANGE_ALTITUDE:
            return FCommandWidgetSchema(
                FCommandWidgetParamSchema(ECommandWidgetParamType::ALTITUDE, 430.0, TEXT("Altitude"), TEXT("m agl"))
            );
        case ECCSimCommand::ORBIT:
            return FCommandWidgetSchema(
                FCommandWidgetParamSchema(ECommandWidgetParamType::FLOAT, 20.0, TEXT("Radius"), TEXT("m")),
                FCommandWidgetParamSchema(ECommandWidgetParamType::FLOAT, 1.0, TEXT("Velocity"), TEXT("m/s")),
                FCommandWidgetParamSchema(ECommandWidgetParamType::LATITUDE, -999.999, TEXT("Latitude")),
                FCommandWidgetParamSchema(ECommandWidgetParamType::LONGITUDE, -999.999, TEXT("Longitude")),
                FCommandWidgetParamSchema(ECommandWidgetParamType::ALTITUDE, 410.5, TEXT("Altitude"), TEXT("m agl"))
            );
        case ECCSimCommand::REPOSITION:
            return FCommandWidgetSchema(
                FCommandWidgetParamSchema(ECommandWidgetParamType::FLOAT, 1.0, TEXT("Velocity"), TEXT("m/s")),
                FCommandWidgetParamSchema(ECommandWidgetParamType::LATITUDE, -999.999, TEXT("Latitude")),
                FCommandWidgetParamSchema(ECommandWidgetParamType::LONGITUDE, -999.999, TEXT("Longitude")),
                FCommandWidgetParamSchema(ECommandWidgetParamType::ALTITUDE, 410.5, TEXT("Altitude"), TEXT("m agl"))
            );
        default:
            return FCommandWidgetSchema();
    }
}


void UCCSimWebsocketMessenger::CallDroneCommandFromGenericParams(ACCSimDrone* Drone, ECCSimCommand Command, FCommandWidgetGenericParams Params) {
    switch (Command) {
        case ECCSimCommand::TAKEOFF:
            Drone->Takeoff(Params.Param1, Params.Param2, Params.Param3);
            break;
        case ECCSimCommand::ARM_TAKEOFF:
            Drone->ArmTakeoff(Params.Param1, Params.Param2, Params.Param3);
            break;
        case ECCSimCommand::CHANGE_ALTITUDE:
            Drone->ChangeAltitude(Params.Param1);
            break;
        case ECCSimCommand::ORBIT:
            Drone->Orbit(Params.Param1, Params.Param2, Params.Param3, Params.Param4, Params.Param5);
            break;
        case ECCSimCommand::REPOSITION:
            Drone->Reposition(Params.Param1, Params.Param3, Params.Param2, Params.Param4);
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("Recieved unsupported command: %i"), static_cast<uint8>(Command))
    }
}

void UCCSimWebsocketMessenger::CallDroneCommandWithNoParams(ACCSimDrone* Drone, ECCSimCommand Command) {
    switch (Command) {
        case ECCSimCommand::ARM:
            Drone->Arm();
            break;
        case ECCSimCommand::DISARM:
            Drone->Disarm();
            break;
        case ECCSimCommand::LAND:
            Drone->Land();
            break;
        case ECCSimCommand::RETURN:
            Drone->Return();
            break;
        case ECCSimCommand::OFFBOARD_ON:
            Drone->OffboardOn();
            break;
        case ECCSimCommand::OFFBOARD_OFF:
            Drone->OffboardOff();
            break;
        case ECCSimCommand::FORMATION:
            Drone->Formation();
            break;
        default:
            break;
    }
}

TArray<FCommandDeclaration> UCCSimWebsocketMessenger::GetCCSimQuickCommands() {
    static TArray<FCommandDeclaration> Commands = {
        FCommandDeclaration(ECCSimCommand::ARM, TEXT("Arm")),
        FCommandDeclaration(ECCSimCommand::DISARM, TEXT("Disarm")),
        FCommandDeclaration(ECCSimCommand::LAND, TEXT("Land")),
        FCommandDeclaration(ECCSimCommand::RETURN, TEXT("Return")),
        /*FCommandDeclaration(ECCSimCommand::OFFBOARD_ON, TEXT("Offboard On")),
        FCommandDeclaration(ECCSimCommand::OFFBOARD_OFF, TEXT("Offboard Off")),
        FCommandDeclaration(ECCSimCommand::FORMATION, TEXT("Formation")),*/
    };
    return Commands;
}

TArray<FCommandDeclaration> UCCSimWebsocketMessenger::GetCCSimConfigurableCommands() {
    static TArray<FCommandDeclaration> Commands = {
        FCommandDeclaration(ECCSimCommand::ARM_TAKEOFF, TEXT("Takeoff")),
        FCommandDeclaration(ECCSimCommand::CHANGE_ALTITUDE, TEXT("Altitude")),
        FCommandDeclaration(ECCSimCommand::ORBIT, TEXT("Orbit")),
        FCommandDeclaration(ECCSimCommand::REPOSITION, TEXT("Reposition")),
    };
    return Commands;
}