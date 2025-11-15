// Copyright (c) 2025 Synetos Aerospace


#include "Core/State/SPManagerBase.h"
#include "Core/State/SPGameState.h"

USPManagerBase::USPManagerBase()
{
    LOG = nullptr;
    GameStateRef = nullptr;
}

void USPManagerBase::Setup_Implementation() {
    GameStateRef = ASPGameState::GetSPGameState(this);
    if (BindToPreferencesUpdates) {
        GameStateRef->Preferences->OnPreferencesUpdated.AddDynamic(this, &USPManagerBase::ApplyPreferencesUpdates);
    }
    PreferencesRef = GameStateRef->Preferences;

    LOG = GameStateRef->Logger;
}

void USPManagerBase::ApplyPreferencesUpdates_Implementation(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences) {}

void USPManagerBase::Teardown_Implementation() {

}

void USPManagerBase::PostSetup() {

}

void USPManagerBase::PreTeardown() {

}

void USPManagerBase::CullRelatedObjects(float MaximumDrawDistance, const FVector& OriginLocationUE) {

}