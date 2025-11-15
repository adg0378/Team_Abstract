// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Delegates/DelegateCombinations.h"
#include "SPLogger.generated.h"

UENUM(BlueprintType)
enum class ELogLevel : uint8
{
	Critical UMETA(DisplayName = "Critical"),
	Error UMETA(DisplayName = "Error"),
	Warning UMETA(DisplayName = "Warning"),
	Info UMETA(DisplayName = "Info"),
	Debug UMETA(DisplayName = "Debug"),
};

USTRUCT(BlueprintType)
struct FLogMessage {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString message;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ELogLevel level;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLogMessageAddedDelegate, const FLogMessage&, LogMessage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLogMessagePoppedDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAlertRecievedDelegate, FString, Message, bool, HasPointOfConcern, FVector, PointOfConcern);

/**
 * Logs events intended for the user to see. Works at runtime unlike UE_LOG
 */
UCLASS()
class SKYPLEXC2_API USPLogger : public UObject
{
	GENERATED_BODY()

public:

	USPLogger();

	UPROPERTY(BlueprintAssignable, Category = "Log")
	FLogMessageAddedDelegate OnLogMessageAdded;

	UPROPERTY(BlueprintAssignable, Category = "Log")
	FLogMessagePoppedDelegate OnLogMessagePopped;

	UPROPERTY(BlueprintAssignable, Category = "Log")
	FAlertRecievedDelegate OnAlertRecieved;

	UFUNCTION(BlueprintCallable)
	void GetLogMessageBuffer(TArray<FLogMessage>& OutBuffer);

	UFUNCTION(BlueprintCallable, Category = "Log")
	void SetCurrentLogLevel(ELogLevel Level);

	UFUNCTION(BlueprintCallable, Category = "Log")
	void SetPrintStringDuration(float Duration);

	UFUNCTION(BlueprintCallable, Category = "Log")
	ELogLevel GetCurrentLogLevel() const;

	UFUNCTION(BlueprintCallable, Category = "Log")
	void Critical(FString Message, FString Origin = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Log")
	void Error(FString Message, FString Origin = TEXT(""), bool EmmitAlert = false);

	UFUNCTION(BlueprintCallable, Category = "Log")
	void Warn(FString Message, FString Origin = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Log")
	void Info(FString Message, FString Origin = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Log")
	void Debug(FString Message, FString Origin = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Log")
	void Alert(FString Message, bool HasPointOfConcern = false, FVector PointOfConcern = FVector::ZeroVector) const;

private:
	ELogLevel CurrentLogLevel = ELogLevel::Info;
	float PrintStringDuration = 5.0f;

	bool ShouldLog(ELogLevel Level) const;

	void Log(ELogLevel Level, FString Message, FString Origin = TEXT(""), bool EmmitAlert = false);

	void BufferToArray(TArray<FLogMessage>& OutArray);

	void Enqueue(FLogMessage Message);

	TQueue<FLogMessage> Buffer;

	TQueue<FLogMessage> BufferAlt;

	int CurrBufferLength = 0;

	int MaxBufferLength = 1000;
};
