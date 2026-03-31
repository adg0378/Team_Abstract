// Fill out your copyright notice in the Description page of Project Settings.


#include "State/Missions/InterestsUtil.h"
#include "Objects/Interests/Interest.h"

InterestsUtil::InterestsUtil()
{
}

InterestsUtil::~InterestsUtil()
{
}

FSPInterestStruct::FSPInterestStruct() :
	ID(-1), TakeoffPoint(nullptr)
{}

FSPInterestStruct::FSPInterestStruct(int32 InID) :
	ID(InID), TakeoffPoint(nullptr)
{}

FPOIDataStruct::FPOIDataStruct()
	: id(-1),
	groupID(-1),
	name(TEXT("unnamed")),
	longLatHeight(FVector())
{
}

FPOIDataStruct::FPOIDataStruct(int32 inID, FString inName, int32 inGroupID, FVector inLongLatHeight, FString Params)
	: id(inID),
	groupID(inGroupID),
	name(inName),
	longLatHeight(inLongLatHeight),
	Params(Params)
{
}

FTakeoffPointDataStruct::FTakeoffPointDataStruct() {}
FTakeoffPointDataStruct::FTakeoffPointDataStruct(int32 InID, FString InName, int32 InGroupID, FVector InLonLatHeight, FString Params)
	: ID(InID),
	GroupID(InGroupID),
	Name(InName),
	LonLatHeight(InLonLatHeight),
	Params(Params)
{
}

FAOIDataStruct::FAOIDataStruct()
	: id(-1),
	groupID(-1),
	name(TEXT("unnamed")),
	longLatHeights(TArray<FVector>())
{
}

FAOIDataStruct::FAOIDataStruct(int32 inID, FString inName, int32 inGroupID, TArray<FVector> inLongLatHeights, FString Params)
	: id(inID),
	groupID(inGroupID),
	name(inName),
	longLatHeights(inLongLatHeights),
	Params(Params)
{
}

FGeoFenceDataStruct::FGeoFenceDataStruct()
	: ID(-1),
	GroupID(-1),
	Name(TEXT("unnamed")),
	LonLatHeights(TArray<FVector>())
{
}

FGeoFenceDataStruct::FGeoFenceDataStruct(int32 inID, FString inName, int32 inGroupID, TArray<FVector> inLongLatHeights, FString Params)
	: ID(inID),
	GroupID(inGroupID),
	Name(inName),
	LonLatHeights(inLongLatHeights),
	Params(Params)
{
}

int32 InterestsUtil::ReverseMapInterestGroup(TMap<int32, FString> Map, FString GroupName) {
	int32 ID = -1;

	for (const TPair<int32, FString>& Entry : Map)
	{
		if (Entry.Value == GroupName)
		{
			ID = Entry.Key;
			break;
		}
	}
	return ID;
}
