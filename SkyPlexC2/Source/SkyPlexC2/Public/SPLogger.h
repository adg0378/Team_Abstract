// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Delegates/DelegateCombinations.h"
#include "Containers/Queue.h"
#include "SPLogger.generated.h"

UENUM(BlueprintType)
enum class ELogLevel : uint8
{
	Critical UMETA(DisplayName = "Critical"),
	DroneAlert UMETA(DisplayName = "DroneAlert"),
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
 * Log messages to the console widget and UE_LOG for debug
 */
UCLASS(Blueprintable, BlueprintType)
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
	void GetLogMessageBuffer(TArray<FLogMessage>& outBuffer);

	UFUNCTION(BlueprintCallable, Category = "Log")
	void SetCurrentLogLevel(ELogLevel level);

	UFUNCTION(BlueprintCallable, Category = "Log")
	void SetPrintStringDuration(float duration);

	UFUNCTION(BlueprintCallable, Category = "Log")
	ELogLevel GetCurrentLogLevel() const;

	UFUNCTION(BlueprintCallable, Category = "Log")
	void Critical(FString message, FString origin = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Log")
	void DroneAlert(FString message, FString origin = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Log")
	void Error(FString message, FString origin = TEXT(""), bool EmmitAlert = false);

	UFUNCTION(BlueprintCallable, Category = "Log")
	void Warn(FString message, FString origin = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Log")
	void Info(FString message, FString origin = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Log")
	void Debug(FString message, FString origin = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Log")
	void LogStorageManagerMessage(ELogLevel level, FString baseMessage, int sqliteReturnCode = 0, FString sqliteErrMsg = TEXT(""), FString origin = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Log")
	void Alert(FString Message, bool HasPointOfConcern = false, FVector PointOfConcern = FVector::ZeroVector) const;
	
private:
	ELogLevel currentLogLevel = ELogLevel::Info;
	float printStringDuration = 5.0f;

	bool ShouldLog(ELogLevel level) const;

	void Log(ELogLevel level, FString message, FString origin = TEXT(""), bool EmmitAlert = false);

	void BufferToArray(TArray<FLogMessage>& outArray);

	void Enqueue(FLogMessage message);

	TQueue<FLogMessage> buffer;

	TQueue<FLogMessage> bufferAlt;

	int currBufferLength = 0;

	int maxBufferLength = 1000;
};
