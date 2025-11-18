// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SPCCSimMessenger.generated.h"

// PX4 commands require uint16 but Unreal Engine currently only supports uint8
// We may have to come up with a workaround in the future but this works alright for now
UENUM(BlueprintType)
enum class ECCSimCommand : uint8 {
	ARM = 0,
	DISARM = 1,
	TAKEOFF = 2,
	LAND = 3,
	CHANGE_ALTITUDE = 4,
	RETURN = 5,
	ORBIT = 6,
	REPOSITION = 7,
	OFFBOARD_ON = 8,
	OFFBOARD_OFF = 9,
	SET_POINT = 10,
	SET_NAV_MODE = 11,
	FORMATION = 12,
	SET_WAYPOINT = 13,
	ARM_TAKEOFF = 14,
};

UENUM(BlueprintType)
enum class ECCSimRecieveChannel : uint8 {
	PONG = 0,
	POSITION = 1,
	STATUS = 2,
	CMD_ACK = 3,
	MISSION_ACK = 4,
	MISSION_FINISHED = 5,
	Unsupported,
	Error,
};

UENUM()
enum class EMissionUploadAction : uint8 {
	DO_NOTHING = 0,
	START = 1,
};

UENUM()
enum class EMissionUploadAckResult : uint8 {
	SUCCESS = 0,
	UPLOAD_FAILURE = 1,
	ACTION_FAILURE = 2,
};

UENUM()
enum class PX4Command : uint8 {
	WAYPOINT = 16,
	RETURN_TO_LAUNCH = 20,
	TAKEOFF = 22,
	CHANGE_SPEED = 178,
};

UENUM()
enum class ECCSimRespondChannel : uint8 {
	PING = 0,
	COMMAND = 1,
	MISSION = 2,
};

USTRUCT()
struct FCCSimDataUnsupported {
	GENERATED_BODY()
};

USTRUCT()
struct FCCSimDataError {
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FCCSimPosition {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double lon;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double lat;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double alt_msl;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double roll;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double pitch;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double yaw;
};

USTRUCT()
struct FCCSimStatus {
	GENERATED_BODY()

public:
	UPROPERTY()
	float battery_remaining;

	UPROPERTY()
	float mission_progress;

	UPROPERTY()
	FString nav_state;

	UPROPERTY()
	FString arming_state;

	UPROPERTY()
	FString latest_arming_reason;

	UPROPERTY()
	FString latest_disarming_reason;

	UPROPERTY()
	FString hil_state;

	UPROPERTY()
	FString gcs_connection_lost;
};

USTRUCT()
struct FCCSimCommandAck {
	GENERATED_BODY()
public:

	UPROPERTY()
	int command;

	UPROPERTY()
	int result;

	UPROPERTY()
	FString result_name;

	UPROPERTY()
	int result_param1;

	UPROPERTY()
	int result_param2;
};

USTRUCT()
struct FCCSimMissionAck {
	GENERATED_BODY()
public:
	UPROPERTY()
	EMissionUploadAckResult result;

	UPROPERTY()
	FString result_name;
};

USTRUCT()
struct FCCSimMissionFinished {
	GENERATED_BODY()
public:
	UPROPERTY()
	int result;
};

using FCCSimDataVariant = TVariant<
	FCCSimDataUnsupported,
	FCCSimDataError,
	FCCSimPosition,
	FCCSimStatus,
	FCCSimCommandAck,
	FCCSimMissionAck,
	FCCSimMissionFinished
>;


UENUM(BlueprintType)
enum class ECommandWidgetParamType : uint8 {
	NONE,
	LATITUDE,
	LONGITUDE,
	ALTITUDE,
	FLOAT,
};


USTRUCT(BlueprintType)
struct FCommandWidgetParamSchema {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ECommandWidgetParamType Type;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float DefaultValue;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Label;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Unit;

	FCommandWidgetParamSchema(
		ECommandWidgetParamType InType = ECommandWidgetParamType::NONE,
		float InDefaultValue = 0.0,
		FString InLabel = TEXT(""),
		FString InUnit = TEXT("")
	)
		: Type(InType), DefaultValue(InDefaultValue), Label(InLabel), Unit(InUnit) {
	}
};


USTRUCT(BlueprintType)
struct FCommandWidgetSchema {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCommandWidgetParamSchema Param1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCommandWidgetParamSchema Param2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCommandWidgetParamSchema Param3;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCommandWidgetParamSchema Param4;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCommandWidgetParamSchema Param5;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCommandWidgetParamSchema Param6;

	FCommandWidgetSchema(
		FCommandWidgetParamSchema InParam1 = FCommandWidgetParamSchema(),
		FCommandWidgetParamSchema InParam2 = FCommandWidgetParamSchema(),
		FCommandWidgetParamSchema InParam3 = FCommandWidgetParamSchema(),
		FCommandWidgetParamSchema InParam4 = FCommandWidgetParamSchema(),
		FCommandWidgetParamSchema InParam5 = FCommandWidgetParamSchema(),
		FCommandWidgetParamSchema InParam6 = FCommandWidgetParamSchema()
	) :
		Param1(InParam1),
		Param2(InParam2),
		Param3(InParam3),
		Param4(InParam4),
		Param5(InParam5),
		Param6(InParam6) {
	}
};


USTRUCT(BlueprintType)
struct FCommandWidgetGenericParams {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double Param1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double Param2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double Param3;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double Param4;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double Param5;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double Param6;
};

USTRUCT(BlueprintType)
struct FCommandDeclaration {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ECCSimCommand Command;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Title;

	FCommandDeclaration(ECCSimCommand InCommand, FString InTitle) : Command(InCommand), Title(InTitle) {}
	FCommandDeclaration() {}
};

/**
 * Assists with parsing and formatting websocket messages
 */
UCLASS()
class SKYPLEXC2_API USPCCSimMessenger : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	

public:

	static FCCSimDataVariant ParseCCSimMessage(const FString& Message, ECCSimRecieveChannel* OutChannel = nullptr);

	static FString MakeCCSimCommandMessage(ECCSimCommand Command, TSharedPtr<FJsonObject> Params);

	static FString MakeResponseMessage(ECCSimRespondChannel Channel, TSharedPtr<FJsonObject> Data);

	static FString MakeMissionUploadMessage(EMissionUploadAction Action, TSharedPtr<FJsonObject> Plan);

	UFUNCTION(BlueprintCallable)
	static TArray<FCommandDeclaration> GetCCSimQuickCommands();

	UFUNCTION(BlueprintCallable)
	static TArray<FCommandDeclaration> GetCCSimConfigurableCommands();

	UFUNCTION(BlueprintCallable)
	static FCommandWidgetSchema GetCommandWidgetSchema(ECCSimCommand Command);

	UFUNCTION(BlueprintCallable)
	static void CallDroneCommandFromGenericParams(class ASPCCSimDrone* Drone, ECCSimCommand Command, FCommandWidgetGenericParams Params);

	UFUNCTION(BlueprintCallable)
	static void CallDroneCommandWithNoParams(ASPCCSimDrone* Drone, ECCSimCommand Command);
};
