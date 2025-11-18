// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "Core/State/SPManagerBase.h"
#include "Delegates/DelegateCombinations.h"
#include "Core/State/SPDatabaseManager.h"
#include "SPMissionManager.generated.h"

/** Stores an interest group's interests */
USTRUCT(BlueprintType)
struct FSPInterestStruct
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 ID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<int32, TObjectPtr<class USPInterest>> Interests;
};

USTRUCT(BlueprintType)
struct FSPMissionProgress {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<int32, float> Progresses;
};

// Update UI when an interest is added
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInterestAddedDelegate, int32, InterestID, int32, MissionID);

// Update UI when an interest is deleted
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInterestDeletedDelegate, int32, InterestID, int32, MissionID);

// Update UI when an interest is updated
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInterestUpdatedDelegate, int32, InterestID, int32, MissionID);

// Update UI when an interest group is updated
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMissionUpdatedDelegate, int32, MissionID);

// Update UI when an interest group is added
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMissionAddedDelegate, int32, MissionID);

// Update UI when an interest group is deleted
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMissionDeletedDelegate, int32, MissionID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMissionFinishedDelegate, int32, MissionID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMissionProgressDelegate, FSPMissionProgress, MissionProgresses);

/**
 * Manages missions and their interests
 */
UCLASS(Blueprintable)
class SKYPLEXC2_API USPMissionManager : public USPManagerBase
{
	GENERATED_BODY()
	
public:

	void Setup_Implementation() override;
	void Teardown_Implementation() override;
	void PreTeardown() override;

	UPROPERTY(BlueprintAssignable, Category = "Missions")
	FMissionFinishedDelegate OnMissionFinished;

	UPROPERTY(BlueprintAssignable, Category = "Missions")
	FMissionProgressDelegate OnMissionProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Interests")
	FInterestAddedDelegate OnInterestAdded;

	UPROPERTY(BlueprintAssignable, Category = "Interests")
	FInterestDeletedDelegate OnInterestDeleted;

	UPROPERTY(BlueprintAssignable, Category = "Interests")
	FInterestUpdatedDelegate OnInterestUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Interests")
	FMissionUpdatedDelegate OnMissionUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Interests")
	FMissionAddedDelegate OnMissionAdded;

	UPROPERTY(BlueprintAssignable, Category = "Interests")
	FMissionDeletedDelegate OnMissionDeleted;

	UFUNCTION(BlueprintCallable, Category = "Interests")
	void GetMissionNames(TArray<FString>& OutKeys);

	// Returns a mapping of group ids to a mapping of interest IDs to interest
	UFUNCTION(BlueprintCallable, Category = "Interests")
	const TMap<int32, FSPInterestStruct>& GetInterests() const;

	UFUNCTION(BlueprintCallable, Category = "Interests")
	TArray<class USPInterest*> GetInterestsInOrder(int32 MissionID) const;

	UFUNCTION(BlueprintCallable, Category = "Interests")
	FString GetMissionName(int MissionID) const;

	UFUNCTION(BlueprintCallable, Category = "Interests")
	const TMap<int32, FSPMissionStruct>& GetMissions() const;

	UFUNCTION(BlueprintCallable, Category = "Interests")
	USPInterest* GetInterest(int32 InterestID, int32 MissionID) const;

	UFUNCTION(BlueprintCallable, Category = "Interests")
	void AddMission(FString Name, int32& MissionID);

	UFUNCTION(BlueprintCallable, Category = "Interests")
	void RenameMission(int32 MissionID, FString NewName);

	UFUNCTION(BlueprintCallable, Category = "Interests")
	void UpdateMissionInteresttOrder(int32 MissionID, TArray<int32> InterestIDs);

	UFUNCTION(BlueprintCallable, Category = "Interests")
	void ValidateMissionName(FString InName, bool& OutIsValid);

	/** If DestroyInterests is false, interests inside the mission will be reassigned to DumpGroupID */
	UFUNCTION(BlueprintCallable, Category = "Interests")
	void DeleteMission(int32 InMissionID, int32 DumpGroupID, bool DestroyInterests = true);

	UFUNCTION(BlueprintCallable, Category = "Interests")
	void AddInterest(USPInterest* Interest, int32 MissionID = -1);

	UFUNCTION(BlueprintCallable, Category = "Interests")
	void RemoveInterest(int32 InterestID, int32 MissionID);

	UFUNCTION(BlueprintCallable, Category = "Interests")
	void ReassignInterest(int32 InterestID, int32 CurrMisisonID, int32 NewMissionID);

	UFUNCTION(BlueprintCallable, Category = "Interests")
	void ReassignInterests(UPARAM(ref) const TMap<int32, int32>& IDToMissionMap, int32 NewGroupID);

	UFUNCTION(BlueprintCallable, Category = "Interests")
	void RenameInterest(int32 InterestID, int32 MissionID, FString NewName);

	UFUNCTION(BlueprintCallable, Category = "Interests")
	void MoveInterest(int32 InterestID, int32 MissionID);

	void CullRelatedObjects(float MaximumDrawDistance, const FVector& OriginLocationUE) override;

	UFUNCTION(BlueprintCallable, Category = "Interests")
	void SetCurrentPlannedMission(int32 NewMissionID = -1);

	// Uploads the mission and then starts the mission
	UFUNCTION(BlueprintCallable, Category = "Missions")
	void StartMission(int32 MissionID);

	/** Unassigns all other swarms and assigns the one passed in */
	UFUNCTION(BlueprintCallable, Category = "Missions")
	void AssignSwarm(int32 SwarmID, int32 MissionID);

	/** Adds the swarm to the current list of swarm assignments */
	UFUNCTION(BlueprintCallable, Category = "Missions")
	void AddAssignSwarm(int32 SwarmID, int32 MissionID);

	UFUNCTION(BlueprintCallable, Category = "Missions")
	void UnassignSwarm(int32 SwarmID, int32 MissionID);

	// this is a hack, we should probably come up with a delegate managed way to report that a mission is
	// finished without relying on searching for the specific swarm ID, but works for now
	void CompleteMission(int32 SwarmID);

private:
	void RenameInterestNoBroadcast(int32 InterestID, int32 MissionID, FString NewName);

	void LoadMissions();

	void LoadInterests();

	void StartMissionProgressTimer();
	void StopMissionProgressTimer();
	void EmitMissionProgress();

	TSharedPtr<FJsonObject> FormatMissionAsJson(int32 MissionID);

	/** Map a group id to its interests */
	UPROPERTY()
	TMap<int32, FSPInterestStruct> Interests;

	/** Map mission id to its name */
	UPROPERTY()
	TMap<int32, FSPMissionStruct> Missions;

	int32 CurrentPlannedMissionID;

	const FString LOG_ORIGIN = TEXT("InterestsManager");

	UPROPERTY()
	FTimerHandle MissionProgressTickHandle;

	const float MissionProgressTimerTickTime = 1.5f;
	bool MissionProgressTimerActive = false;
};
