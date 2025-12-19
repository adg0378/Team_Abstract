// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "BasicDrone.h"
#include "IWebSocket.h"
#include "CLIUtility.h"
#include "CCSimDrone.generated.h"

// forward declarations
class USPLogger;

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API ACCSimDrone : public ABasicDrone
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void Setup(FString InHost, int32 InPort, USPLogger* InLog);

	virtual void DestroySelf_Implementation() override;
	virtual void BeginDestroy() override;

	virtual void SetPreferences(const FCCSimPreferencesStruct& InPrefs, const FDronePreferencesStruct& InDronePrefs) override;

	virtual void SetStatus(const FCCSimStatus& InStatus) override;
	virtual void SetPosition(const FCCSimPosition& InPosition) override;

	UFUNCTION(BlueprintCallable)
	void Arm();

	UFUNCTION(BlueprintCallable)
	void Disarm();

	UFUNCTION(BlueprintCallable)
	void Takeoff(float Lat, float Lon, float Altitude);

	UFUNCTION(BlueprintCallable)
	void ArmTakeoff(float Lat, float Lon, float Altitude);

	UFUNCTION(BlueprintCallable)
	void Land();

	UFUNCTION(BlueprintCallable)
	void ChangeAltitude(float Altitude);

	UFUNCTION(BlueprintCallable)
	void Return();

	UFUNCTION(BlueprintCallable)
	void Orbit(float Radius, float Velocity, float Lat, float Lon, float Altitude);

	UFUNCTION(BlueprintCallable)
	void Reposition(float Velocity, float Lon, float Lat, float Altitude);

	UFUNCTION(BlueprintCallable)
	void OffboardOn();

	UFUNCTION(BlueprintCallable)
	void OffboardOff();

	UFUNCTION(BlueprintCallable)
	void SetPoint(FVector Point);

	UFUNCTION(BlueprintCallable)
	void SetWaypoint(float HoldTime, float AcceptanceRadius, float PassRadius, float Yaw, float Lat, float Lon, float Altitude);

	UFUNCTION(BlueprintCallable)
	void Formation();

	void UploadMission(const FString& Mission);

	UCLIUtility::CLIProcs PX4Processes;

protected:
	static const bool Simulated = true;

private:
	void InitializeWebsocket(FString InHost, int32 InPort);
	void ConnectWebsocket();

	// Only call this to retry connecting after seting up and failing the first time
	void _RetryConnectWebsocket();
	void CloseWebsocket();
	void HandleMessage(const FString& Message);

	bool WebsocketRetryEnabled = true;

	void SendCommand(FString Message);
	bool IsAwaitingCommand = false;

	void SampleAltitude();

	TSharedPtr<IWebSocket> Websocket = nullptr;
	FString Host;
	int32 Port;
	bool Connected = false;

	bool ShowAltitudeReferenceLineTraces = false;
	float AltitudeRefreshDistance = 10.0;
	float StatusUpdatesSinceLastSample = 0;
	FVector PosAtLastAltitudeSample = FVector::ZeroVector;
	float TilesetHeightM = 0.0;

	UPROPERTY()
	TObjectPtr<USPLogger> LOG;
};
