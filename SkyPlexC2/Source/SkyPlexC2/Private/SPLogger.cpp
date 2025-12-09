// Fill out your copyright notice in the Description page of Project Settings.


#include "SPLogger.h"

static void UELOG_(FLogMessage message) {
	switch (message.level)
	{
		case ELogLevel::Critical:
		case ELogLevel::DroneAlert:
		case ELogLevel::Error:
			UE_LOG(LogTemp, Error, TEXT("%s"), *message.message);
			break;

		case ELogLevel::Warning:
			UE_LOG(LogTemp, Warning, TEXT("%s"), *message.message);
			break;

		case ELogLevel::Info:
			UE_LOG(LogTemp, Display, TEXT("%s"), *message.message);
			break;

		case ELogLevel::Debug:
			UE_LOG(LogTemp, Verbose, TEXT("%s"), *message.message);
			break;

		default:
			UE_LOG(LogTemp, Display, TEXT("%s"), *message.message);
	}
}

static FLinearColor LogLevelToColor(ELogLevel level) {
	switch (level)
	{
		case ELogLevel::Critical:
		case ELogLevel::DroneAlert:
		case ELogLevel::Error:
			return FLinearColor::Red;

		case ELogLevel::Warning:
			return FLinearColor::Yellow;

		case ELogLevel::Info:
			return FLinearColor::White;

		case ELogLevel::Debug:
			return FLinearColor::Blue;

		default:
			return FLinearColor::White;
	}
}

static FString GetLogMsgFmt(ELogLevel level, FString origin) {
	FString levelString;
	switch (level)
	{
		case ELogLevel::Critical:
			levelString = TEXT("Critical Error");
			break;
			
		case ELogLevel::DroneAlert:
			levelString = TEXT("Drone Alert");
			break;

		case ELogLevel::Error:
			levelString = TEXT("Error");
			break;

		case ELogLevel::Warning:
			levelString = TEXT("Warning");
			break;

		case ELogLevel::Info:
			levelString = TEXT("Info");
			break;

		case ELogLevel::Debug:
			levelString = TEXT("Debug");
			break;

		default:
			levelString = TEXT("Info");
			break;
	}

	if (origin.IsEmpty()) {
		return FString::Printf(TEXT("[{0}] %s:{1} '{2}'"), *levelString);
	}
	return FString::Printf(TEXT("[{0}] %s: {1}: '{2}'"), *levelString);
}

USPLogger::USPLogger()
{

}

// log level should never go below error. Critical, DroneAlert, etc are all types of errors
void USPLogger::SetCurrentLogLevel(ELogLevel level) {
	if (static_cast<uint8>(level) <= static_cast<uint8>(ELogLevel::Error)) {
		currentLogLevel = ELogLevel::Error;
	}
	else {
		currentLogLevel = level;
	}
}

ELogLevel USPLogger::GetCurrentLogLevel() const {
	return currentLogLevel;
}

void USPLogger::SetPrintStringDuration(float duration) {
	if (duration >= 1.0f) {
		printStringDuration = duration;
	}
	else {
		printStringDuration = 1.0f;
	}
}

void USPLogger::DroneAlert(FString message, FString origin) {
	if (ShouldLog(ELogLevel::DroneAlert)) {
		Log(ELogLevel::DroneAlert, message, origin);
	}
}

void USPLogger::Critical(FString message, FString origin) {
	if (ShouldLog(ELogLevel::Critical)) {
		Log(ELogLevel::Critical, message, origin);
	}
}

void USPLogger::Error(FString message, FString origin, bool EmmitAlert) {
	if (ShouldLog(ELogLevel::Error)) {
		Log(ELogLevel::Error, message, origin, EmmitAlert);
	}
}

void USPLogger::Warn(FString message, FString origin) {
	if (ShouldLog(ELogLevel::Warning)) {
		Log(ELogLevel::Warning, message, origin);
	}
}

void USPLogger::Info(FString message, FString origin) {
	if (ShouldLog(ELogLevel::Info)) {
		Log(ELogLevel::Info, message, origin);
	}
}

void USPLogger::Debug(FString message, FString origin) {
	if (ShouldLog(ELogLevel::Debug)) {
		Log(ELogLevel::Debug, message, origin);
	}
}

void USPLogger::LogStorageManagerMessage(ELogLevel level, FString baseMessage, int sqliteReturnCode, FString sqliteErrMsg, FString origin) {
	if (ShouldLog(level)) {
		Log(level, baseMessage, origin);
	}
	if (sqliteErrMsg.IsEmpty() || sqliteReturnCode != 0) {
		Log(ELogLevel::Error, FString::Format(*FString("{0} : return code {1}"), { *sqliteErrMsg, sqliteReturnCode }), origin);
	}
}

void USPLogger::Log(ELogLevel level, FString message, FString origin, bool EmmitAlert) {
	if (IsEngineExitRequested()) {
		return;
	}

	FDateTime currentDateTime = FDateTime::Now();
	FLogMessage M{.message = FString::Format(*GetLogMsgFmt(level, origin), { *currentDateTime.ToString(TEXT("%H:%M:%S")), *origin, *message }), .level = level,};

	if (GetWorld()->bIsTearingDown) {
		return;
	}

	if (IsInGameThread()) {
		Enqueue(M);
		OnLogMessageAdded.Broadcast(M);
		if (EmmitAlert) {
			OnAlertRecieved.Broadcast(message, false, FVector::ZeroVector);
		}
		UELOG_(M);
	}
	else {
		AsyncTask(ENamedThreads::GameThread, [this, M, message, EmmitAlert]() {
			Enqueue(M);
			OnLogMessageAdded.Broadcast(M);
			if (EmmitAlert) {
				OnAlertRecieved.Broadcast(message, false, FVector::ZeroVector);
			}
			UELOG_(M);
		});
	}
}

void USPLogger::Enqueue(FLogMessage message) {
	if (buffer.IsEmpty()) {
		bufferAlt.Enqueue(message);
	}
	else {
		buffer.Enqueue(message);
	}

	if (currBufferLength == maxBufferLength) {
		if (buffer.IsEmpty()) {
			bufferAlt.Pop();
		}
		else {
			buffer.Pop();
		}
		OnLogMessagePopped.Broadcast();
	}
	else {
		currBufferLength++;
	}
}

bool USPLogger::ShouldLog(ELogLevel level) const {
	return static_cast<uint8>(level) <= static_cast<uint8>(currentLogLevel);
}

void USPLogger::GetLogMessageBuffer(TArray<FLogMessage>& outBuffer) {
	BufferToArray(outBuffer);
}

void USPLogger::BufferToArray(TArray<FLogMessage>& outArray) {
	outArray.Empty();

	FLogMessage m;
	if (buffer.IsEmpty()) {
		while (bufferAlt.Dequeue(m)) {
			buffer.Enqueue(m);
			outArray.Add(m);
		}
	} else {
		while (buffer.Dequeue(m)) {
			bufferAlt.Enqueue(m);
			outArray.Add(m);
		}
	}
}

void USPLogger::Alert(FString Message, bool HasPointOfConcern, FVector PointOfConcern) const {
	if (IsInGameThread()) {
		OnAlertRecieved.Broadcast(Message, HasPointOfConcern, PointOfConcern);
	}
	else {
		AsyncTask(ENamedThreads::GameThread, [this, Message, HasPointOfConcern, PointOfConcern]() {
			OnAlertRecieved.Broadcast(Message, HasPointOfConcern, PointOfConcern);
		});
	}
}