// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/SPLogger.h"
#include "Core/SPPreferences.h"
#include "SPManagerBase.generated.h"


/**
 * Base class for state managers
 */
UCLASS(Blueprintable)
class SKYPLEXC2_API USPManagerBase : public UObject
{
	GENERATED_BODY()
	
public:
	USPManagerBase();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Setup();
	virtual void Setup_Implementation();

	/** Runs after all manager `Setup` methods have been called */
	UFUNCTION()
	virtual void PostSetup();

	/** Runs before all managers are torn down */
	UFUNCTION()
	virtual void PreTeardown();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Teardown();
	virtual void Teardown_Implementation();

	UFUNCTION()
	virtual void CullRelatedObjects(float MaximumDrawDistance, const FVector& OriginLocationUE);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ApplyPreferencesUpdates(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences);
	virtual void ApplyPreferencesUpdates_Implementation(FSPPreferencesStruct PrevPreferences, FSPPreferencesStruct NewPreferences);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Logger")
	TObjectPtr<USPLogger> LOG;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	TObjectPtr<class ASPGameState> GameStateRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Managers")
	TObjectPtr<USPPreferences> PreferencesRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Managers")
	bool BindToPreferencesUpdates = false;
};
