// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Core/State/SPManagerBase.h"
#include "Delegates/DelegateCombinations.h"
#include "SPDroneManager.generated.h"

USTRUCT(BlueprintType)
struct FSPSwarmStruct
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<int32, TObjectPtr<class ASPBaseDrone>> Drones;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDroneAddedDelegate, int32, DroneID, int32, SwarmID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDroneUpdatedDelegate, int32, DroneID, int32, SwarmID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDroneDeletedDelegate, int32, DroneID, int32, SwarmID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSwarmAddedDelegate, int32, SwarmID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSwarmUpdatedDelegate, int32, SwarmID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSwarmDeletedDelegate, int32, SwarmID);

/**
 * Drone manager
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API USPDroneManager : public USPManagerBase
{
	GENERATED_BODY()
	
public:

	USPDroneManager();

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
	TSubclassOf<ASPBaseDrone> DroneToSpawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<class ASPCCSimDrone> CCSimDroneToSpawn;

	void Setup_Implementation() override;
	void Teardown_Implementation() override;
	void ApplyPreferencesUpdates_Implementation(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences) override;

	UFUNCTION(BlueprintCallable)
	FString GetSwarmName(const int32 SwarmID) const;

	UFUNCTION(BlueprintCallable)
	ASPBaseDrone* GetDrone(const int32 DroneID, const int32 SwarmID) const;

	UFUNCTION(BlueprintCallable)
	const TMap<int32, FSPSwarmStruct>& GetDrones() const;

	TArray<const FSPSwarmStruct*> GetSwarmDrones(const TArray<int32>& SwarmIDs) const;

	UFUNCTION(BlueprintCallable)
	int32 GetUnassignedSwarmID() const;

	UFUNCTION(BlueprintCallable)
	const TMap<int32, FString>& GetSwarms() const;

	UFUNCTION(BlueprintCallable)
	int32 GetNumSimDrones() const;

	UFUNCTION(BlueprintCallable)
	void AddSwarm(FString Name, int32& OutSwarmID);

	UFUNCTION(BlueprintCallable)
	void RenameSwarm(int32 SwarmID, FString NewName);

	UFUNCTION(BlueprintCallable)
	void DeleteSwarm(int32 InSwarmID, bool DisconnectDrones = false, int32 DumpSwarmID = -1);

	UFUNCTION(BlueprintCallable)
	void AddDrone(int32 SwarmID = -1, bool IsCCSim = false);

	UFUNCTION(BlueprintCallable)
	void ReassignDrone(int32 DroneID, int32 CurrSwarmID, int32 NewSwarmID);

	UFUNCTION(BlueprintCallable)
	void ReassignDrones(UPARAM(ref) const TMap<int32, int32>& IDToSwarmMap, int32 NewSwarmID);

	UFUNCTION(BlueprintCallable)
	void RenameDrone(int32 DroneID, int32 SwarmID, FString NewName);

	UFUNCTION(BlueprintCallable)
	void RemoveDrone(int32 DroneID, int32 SwarmID);


	virtual void CullRelatedObjects(float MaximumDrawDistance, const FVector& OriginLocationUE) override;

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
	TObjectPtr<class USPCLIUtility> cli;

	FString CCSimHost = "127.0.0.1";
	int32 CCSimPort = 8765;
	int32 NumCCSimDrones = 0;

	int32 UnassignedID = -1;

	const FString UNASSIGNED = "Default";
	const FString LOG_ORIGIN = TEXT("SPDroneManager");
};
