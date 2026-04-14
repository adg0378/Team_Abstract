// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "State/SPManager.h"
#include "IWebSocket.h"
#include <vector>
#include "Delegates/DelegateCombinations.h"
#include "CLIUtility.h"
#include "SPDroneManager.generated.h"

// forward declarations
class ABasicDrone;
class ACCSimDrone;

USTRUCT(BlueprintType)
struct FSPSwarmStruct
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<int32, TObjectPtr<ABasicDrone>> Drones;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDroneAddedDelegate, int32, DroneID, int32, SwarmID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDroneUpdatedDelegate, int32, DroneID, int32, SwarmID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDroneDeletedDelegate, int32, DroneID, int32, SwarmID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSwarmAddedDelegate, int32, SwarmID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSwarmUpdatedDelegate, int32, SwarmID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSwarmDeletedDelegate, int32, SwarmID);

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API USPDroneManager : public USPManager
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FDroneAddedDelegate OnDroneAdded;

	UPROPERTY(BlueprintAssignable)
	FDroneUpdatedDelegate OnDroneUpdated;

	UPROPERTY(BlueprintAssignable)
	FDroneDeletedDelegate OnDroneDeleted;

	UPROPERTY(BlueprintAssignable)
	FSwarmAddedDelegate OnSwarmAdded;

	UPROPERTY(BlueprintAssignable)
	FSwarmUpdatedDelegate OnSwarmUpdated;

	UPROPERTY(BlueprintAssignable)
	FSwarmDeletedDelegate OnSwarmDeleted;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<ABasicDrone> DroneToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<ACCSimDrone> CCSimDroneToSpawn;

	int32 selectedDroneId = -1;

	ABasicDrone *getSelectedDrone();

	void Setup_Implementation(bool &outSuccess) override;
	void Teardown_Implementation() override;
	void ApplyPreferencesUpdates_Implementation(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences) override;

	UFUNCTION(BlueprintCallable)
	FString GetSwarmName(const int32 SwarmID) const;

	UFUNCTION(BlueprintCallable)
	ABasicDrone *GetDrone(const int32 DroneID, const int32 SwarmID) const;

	UFUNCTION(BlueprintCallable)
	const TMap<int32, FSPSwarmStruct> &GetDrones() const;

	TArray<const FSPSwarmStruct *> GetSwarmDrones(const TArray<int32> &SwarmIDs) const;
	int GetNumSwarmDrones(const TArray<int32> &SwarmIDs) const;

	UFUNCTION(BlueprintCallable)
	int32 GetUnassignedSwarmID() const;

	UFUNCTION(BlueprintCallable)
	const TMap<int32, FString> &GetSwarms() const;

	UFUNCTION(BlueprintCallable)
	int32 GetNumSimDrones() const;

	UFUNCTION(BlueprintCallable)
	void AddSwarm(FString Name, int32 &OutSwarmID);

	UFUNCTION(BlueprintCallable)
	void RenameSwarm(int32 SwarmID, FString NewName);

	UFUNCTION(BlueprintCallable)
	void DeleteSwarm(int32 InSwarmID, bool DisconnectDrones = false, int32 DumpSwarmID = -1);

	UFUNCTION(BlueprintCallable)
	void AddDrone(int32 SwarmID = -1, bool IsCCSim = false);

	UFUNCTION(BlueprintCallable)
	void ReassignDrone(int32 DroneID, int32 CurrSwarmID, int32 NewSwarmID);

	UFUNCTION(BlueprintCallable)
	void ReassignDrones(UPARAM(ref) const TMap<int32, int32> &IDToSwarmMap, int32 NewSwarmID);

	UFUNCTION(BlueprintCallable)
	void RenameDrone(int32 DroneID, int32 SwarmID, FString NewName);

	UFUNCTION(BlueprintCallable)
	void RemoveDrone(int32 DroneID, int32 SwarmID);

	void setSelectedDroneId(int32 DroneID);

	virtual void CullRelatedObjects(float MaximumDrawDistance, const FVector &OriginLocationUE, bool FromFlyTo = false) override;

	UFUNCTION(BlueprintCallable)
	int32 GetCurrPortNum();

private:
	void LoadSwarms();

	UPROPERTY()
	// make this a map of int32s to ABasicDrone
	TMap<int32, FSPSwarmStruct> Drones;

	UPROPERTY()
	// make this map to a struct containing swarm name and drone ids in the swarm
	TMap<int32, FString> Swarms;

	UPROPERTY()
	UCLIUtility *cli;

	FString CCSimHost = "127.0.0.1";
	int32 CCSimPort = 8765;
	int32 NumCCSimDrones = 0;

	int32 UnassignedID = -1;
	bool StartedDDSBridge = false;

	const FString UNASSIGNED = "Default";
	const FString LOG_ORIGIN = TEXT("SPDroneManager");
};