// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SPInteractionInterface.generated.h"

// Widget to display in the interaction box
UENUM(BlueprintType)
enum class EKeyValDisplayType : uint8
{
	UneditableText UMETA(DisplayName = "Uneditable Text"),
	DeleteInterestButton UMETA(DisplayName = "DeleteInterestButton")
};

// Value to pass to the widget displayed in the interaction box
USTRUCT(BlueprintType)
struct FInteractionBoxValue {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EKeyValDisplayType displayType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText value;

	bool operator==(const FInteractionBoxValue& Other) const
	{
		return displayType == Other.displayType &&
			value.ToString().Equals(Other.value.ToString(), ESearchCase::CaseSensitive);
	}
};

UINTERFACE(MinimalAPI)
class USPInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/*
	Any class that manages instances of `Interactable` and wishes to log data in the interaction widget
	must implement this interface.

	All methods must be implemented. If an implementor does not wish to support a linked provider, they should
	return nullptr instead of managing a linked provider. However, if an implementor may be managed by
	another linked provider, that provider will not be able to pass info to Interactable unless the middleclass
	maintains a link to its owner.

	Given that users can delete what they have selected, this method also provides a shared DestroySelf method that will call up the tree.

	Children complete any necessary actions, then call their LinkedProvider's DestroySelf method. This pattern continues until the root.
	The root performs its necessary actions, then deletes child, so on and so forth.

	Nothing should rely on the instance once the LinkedProvider's DestroySelf method is called.
*/
class SKYPLEXC2_API ISPInteractionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetInteractionBoxKeyVals(TMap<FString, FInteractionBoxValue>& OutKeyVals);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetInteractionBoxTitle(FText& OutTitle);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnInteractionBoxTitleChanged(const FText& NewTitle);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ToggleCull(bool IsCulled);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetLinkedProvider(const TScriptInterface<USPInteractionInterface>& InProvider);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	TScriptInterface<USPInteractionInterface> GetLinkedProvider() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DestroySelf();
};
