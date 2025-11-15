// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SPGameState.generated.h"

/**
 * Stores managers, plugins, and other commonly accessed references
 */
UCLASS()
class SKYPLEXC2_API ASPGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class ACesium3DTileset> CesiumTileset;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class ACesiumGeoreference> CesiumGeoreference;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UWorld> SPWorld;

	UFUNCTION(BlueprintCallable, Category = "SPGameState", meta = (WorldContext = "WorldContextObject"))
	static ASPGameState* GetSPGameState(const UObject* WorldContextObject);

protected:
	virtual void BeginPlay() override;
};
