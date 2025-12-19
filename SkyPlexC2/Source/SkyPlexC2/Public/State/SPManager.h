// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SPLogger.h"
#include "SPGameState.h"
#include "SPEnvConstants.h"
#include "SPManager.generated.h"


// Forward declarations
struct FSPPreferencesStruct;
class UPreferencesManager;
class ASPGameState;
class USPStorageManager;


/**
 * Base game state class that initializes logger and game state ref for subclasses
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API USPManager : public UObject
{
	GENERATED_BODY()
	
public:
	USPManager();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Setup(bool& outSuccess);
	virtual void Setup_Implementation(bool& outSuccess);

	/** Runs after all the manager's `Setup` methods have been called */
	UFUNCTION()
	virtual void PostSetup();

	/** Runs before all the manager's are torn down */
	UFUNCTION()
	virtual void PreTeardown();

	UFUNCTION()
	virtual void CullRelatedObjects(float MaximumDrawDistance, const FVector& OriginLocationUE, bool FromFlyTo = false);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Teardown();
	virtual void Teardown_Implementation();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ApplyPreferencesUpdates(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences);
	virtual void ApplyPreferencesUpdates_Implementation(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences);

	UFUNCTION(BlueprintCallable)
	void getLOG(USPLogger*& outLOG) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Managers")
	TObjectPtr<USPLogger> LOG;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<ASPGameState> gameStateRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Managers")
	TObjectPtr<UPreferencesManager> PreferencesManagerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Managers")
	TObjectPtr<USPStorageManager> StorageManagerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Managers")
	bool BindToPreferencesUpdates = false;
};
