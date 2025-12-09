// Fill out your copyright notice in the Description page of Project Settings.


#include "State/SPManager.h"
#include "SPUtility.h"
#include "State/PreferencesManager.h"
#include "SPLogger.h"
#include "State/SPGameState.h"


USPManager::USPManager()
{
    LOG = nullptr;
    gameStateRef = nullptr;
}

void USPManager::Setup_Implementation(bool& outSuccess) {
    gameStateRef = USPUtility::GetSPGameState(this);
    if (BindToPreferencesUpdates) {
        gameStateRef->PreferencesManager->OnPreferencesUpdated.AddDynamic(this, &USPManager::ApplyPreferencesUpdates);
    }
    PreferencesManagerRef = gameStateRef->PreferencesManager;
    StorageManagerRef = gameStateRef->storageManagerRef;

    LOG = gameStateRef->loggerRef;

    if (LOG == nullptr || gameStateRef == nullptr) {
        outSuccess = false;
    }
    else {
        outSuccess = true;
    }
}

void USPManager::ApplyPreferencesUpdates_Implementation(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences) {}

void USPManager::getLOG(USPLogger*& outLOG) const {
    outLOG = LOG;
}

void USPManager::Teardown_Implementation() {

}

void USPManager::PostSetup() {

}

void USPManager::PreTeardown() {

}

void USPManager::CullRelatedObjects(float MaximumDrawDistance, const FVector& OriginLocationUE, bool FromFlyTo) {

}