// Fill out your copyright notice in the Description page of Project Settings.


#include "SPUtility.h"
#include "State/SPGameState.h"
#include "SPLogger.h"
#include "Player/SPPlayerController.h"
#include "Objects/Interactable.h"
#include <Engine/Engine.h>
#include <CesiumGlobeAnchorComponent.h>
#include <map>

const float TRACE_DISTANCE = 20000000.0;

TMap<EState, UMaterialInstance*> USPUtility::stateMaterials;

TEnumAsByte<ECollisionChannel> USPUtility::landscapeTraceChannel;

TEnumAsByte<ETraceTypeQuery> USPUtility::landscapeTraceType;

ECollisionChannel USPUtility::GetLandscapeTraceChannel() {
	if (!landscapeTraceChannel) {
		BuildTraceChannels();
	}
	return landscapeTraceChannel;
}

ETraceTypeQuery USPUtility::GetLandscapeTraceType() {
	if (!landscapeTraceChannel) {
		BuildTraceChannels();
	}
	return landscapeTraceType;
}

void USPUtility::BuildTraceChannels() {
	FName landscapeProfileName = "Landscape";
	ECollisionChannel collisionChannel;
	FCollisionResponseParams responseParams;
	if (UCollisionProfile::GetChannelAndResponseParams(landscapeProfileName, collisionChannel, responseParams)) {
		landscapeTraceChannel = collisionChannel;
		landscapeTraceType = UEngineTypes::ConvertToTraceType(collisionChannel);
	}
}

UWorld* USPUtility::GetWorldContext(UObject* worldContextObject) {
	return worldContextObject->GetWorld();

}

ASPGameState* USPUtility::GetSPGameState(const UObject* worldContextObject) {
	UWorld* world = worldContextObject->GetWorld();
	if (world) {
		AGameStateBase* gameState = UGameplayStatics::GetGameState(world);
		if (gameState) {
			ASPGameState* spGameState = Cast<ASPGameState>(gameState);
			return spGameState;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Failed to get SPGameState"))
	return nullptr;
}

ASPPlayerController* USPUtility::GetSPPlayerController(UObject* worldContextObject) {
	UWorld* world = worldContextObject->GetWorld();
	if (world) {
		APlayerController* controller = UGameplayStatics::GetPlayerController(world, 0);
		if (controller) {
			ASPPlayerController* spController = Cast<ASPPlayerController>(controller);
			return spController;
		}
	}
	return nullptr;
}

APlayerController* USPUtility::GetRegularPlayerController(UObject* worldContextObject) {
	UWorld* world = worldContextObject->GetWorld();
	if (world) {
		APlayerController* controller = UGameplayStatics::GetPlayerController(world, 0);
		return controller;
	}
	return nullptr;
}


USPLogger* USPUtility::GetLogger(UObject* worldContextObject) {
	ASPGameState* state = GetSPGameState(worldContextObject);
	return state->loggerRef;
}

FText USPUtility::StateToDisplayName(EState state) {
	return UEnum::GetDisplayValueAsText(state);
}

UMaterialInstance* USPUtility::StateToMaterial(UObject* worldContextObject, EState state) {
	if (stateMaterials.Num() == 0) {
		BuildStateMaterialsMap(worldContextObject);
	}
	return *stateMaterials.Find(state);
}

void USPUtility::BuildStateMaterialsMap(UObject* worldContextObject) {
	FString baseMaterialsPath = TEXT("MaterialInstanceConstant'/Game/Materials/{0}'");

	std::map<EState, FString> materials = {
		{EState::Unassigned, "M_ClickableUnassigned.M_ClickableUnassigned"},
		{EState::Paused, "M_ClickablePaused.M_ClickablePaused"},
		{EState::Selected, "M_ClickableSelected.M_ClickableSelected"},
		{EState::Error, "M_ClickableInvalid.M_ClickableInvalid"},
		{EState::Running, "M_ClickableRunning.M_ClickableRunning"},
	};

	for (const auto& pair : materials) {
		UMaterialInstance* loadedMaterial = Cast<UMaterialInstance>(StaticLoadObject(UMaterialInstance::StaticClass(), nullptr, *FString::Format(*baseMaterialsPath, { *pair.second })));

		if (loadedMaterial)
		{
			stateMaterials.Add(pair.first, loadedMaterial);
		}
		else if (worldContextObject) {
			USPLogger* log = GetLogger(worldContextObject);
			log->Error(FString::Format(*FString("Unable to load material '{0}' for state '{1}'"), { *pair.second, *FString::FromInt((int32)pair.first) }));
		}
	}
}

void USPUtility::MoveActorToMousePosition(UObject* worldContextObject, AActor* actor, FVector mouseOffset, bool moveAllSelected) {
	UWorld* world = worldContextObject->GetWorld();

	if (!actor || !world) {
		return;
	}

	FVector worldLocationStart, worldDirection;
	if (UGameplayStatics::GetPlayerController(world, 0)->DeprojectMousePositionToWorld(worldLocationStart, worldDirection)) {
		 FVector end = worldLocationStart + (worldDirection * TRACE_DISTANCE);
		 FHitResult hit;
		 world->LineTraceSingleByChannel(hit, worldLocationStart, end, GetLandscapeTraceChannel());
		 if (hit.bBlockingHit) {
			 ASPGameState* gameState = GetSPGameState(worldContextObject);
			 bool isMultipleSelected;
			 gameState->IsMultipleSelected(isMultipleSelected);
			 if (isMultipleSelected && moveAllSelected) {
				 FVector movedOffset = actor->GetActorLocation() - (hit.Location + mouseOffset);
				 for (AInteractable* a : gameState->selectedActors) {
					 if (a->userPlacementEnabled) {
						 SetActorLocationAboveCesiumTerrain(worldContextObject, a, movedOffset);
					 }
				 }
			 }
			 else {
				 FVector movedOffset = actor->GetActorLocation() - (hit.Location + mouseOffset);
				 SetActorLocationAboveCesiumTerrain(worldContextObject, actor, movedOffset);
			 }
		 }
	}
}

void USPUtility::SetActorLocationAboveCesiumTerrain(UObject* worldContextObject, AActor* actor, FVector offsetFromCurrentLocation) {
	UWorld* world = worldContextObject->GetWorld();
	actor->SetActorEnableCollision(false);

	FVector startLocation = actor->GetActorLocation() - offsetFromCurrentLocation;
	FVector newLocation = startLocation;
	FVector upVector = actor->GetActorUpVector();
	FHitResult upHitResult, downHitResult;
	bool upHit = world->LineTraceSingleByChannel(upHitResult, startLocation, startLocation + (upVector * TRACE_DISTANCE), GetLandscapeTraceChannel());
	bool downHit = world->LineTraceSingleByChannel(downHitResult, startLocation, startLocation + (-upVector * TRACE_DISTANCE), GetLandscapeTraceChannel());
	if (upHit && downHit) {
		//DrawDebugLine(world, newLocation, upHitResult.ImpactPoint, FColor::Red, false, 5.0f, 0, 1.0f);
		//DrawDebugLine(world, newLocation, downHitResult.ImpactPoint, FColor::Red, false, 5.0f, 0, 1.0f);

		// If uphit is closer than downhit, use uphit
		const FVector upDelta = upHitResult.ImpactPoint - startLocation;
		const FVector downDelta = downHitResult.ImpactPoint - startLocation;
		if (upDelta.Size() <= downDelta.Size()) {
			newLocation = startLocation + upVector * FVector::DotProduct(upDelta, upVector);
		}
		else {
			newLocation = startLocation + upVector * FVector::DotProduct(downDelta, upVector);
		}
	}
	else if (upHit) {
		//DrawDebugLine(world, newLocation, upHitResult.ImpactPoint, FColor::Red, false, 5.0f, 0, 2.0f);
		const FVector upDelta = upHitResult.ImpactPoint - startLocation;
		newLocation = startLocation + upVector * FVector::DotProduct(upDelta, upVector);
	}
	else if (downHit) {
		//DrawDebugLine(world, newLocation, downHitResult.ImpactPoint, FColor::Red, false, 5.0f, 0, 2.0f);
		const FVector downDelta = downHitResult.ImpactPoint - startLocation;
		newLocation = startLocation + upVector * FVector::DotProduct(downDelta, upVector);
	}
	actor->SetActorLocation(newLocation);
	actor->SetActorEnableCollision(true);
}

