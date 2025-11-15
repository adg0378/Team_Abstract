// Copyright (c) 2025 Synetos Aerospace


#include "Core/SPLogger.h"

static void UELOG_(FLogMessage message) {
	switch (message.level)
	{
	case ELogLevel::Critical:
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
		CurrentLogLevel = ELogLevel::Error;
	}
	else {
		CurrentLogLevel = level;
	}
}

ELogLevel USPLogger::GetCurrentLogLevel() const {
	return CurrentLogLevel;
}

void USPLogger::SetPrintStringDuration(float duration) {
	if (duration >= 1.0f) {
		PrintStringDuration = duration;
	}
	else {
		PrintStringDuration = 1.0f;
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

void USPLogger::Log(ELogLevel level, FString message, FString origin, bool EmmitAlert) {
	FDateTime currentDateTime = FDateTime::Now();
	FLogMessage M{ .message = FString::Format(*GetLogMsgFmt(level, origin), { *currentDateTime.ToString(TEXT("%H:%M:%S")), *origin, *message }), .level = level, };
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
	if (Buffer.IsEmpty()) {
		BufferAlt.Enqueue(message);
	}
	else {
		Buffer.Enqueue(message);
	}

	if (CurrBufferLength == MaxBufferLength) {
		if (Buffer.IsEmpty()) {
			BufferAlt.Pop();
		}
		else {
			Buffer.Pop();
		}
		OnLogMessagePopped.Broadcast();
	}
	else {
		CurrBufferLength++;
	}
}

bool USPLogger::ShouldLog(ELogLevel level) const {
	return static_cast<uint8>(level) <= static_cast<uint8>(CurrentLogLevel);
}

void USPLogger::GetLogMessageBuffer(TArray<FLogMessage>& outBuffer) {
	BufferToArray(outBuffer);
}

void USPLogger::BufferToArray(TArray<FLogMessage>& outArray) {
	outArray.Empty();

	FLogMessage m;
	if (Buffer.IsEmpty()) {
		while (BufferAlt.Dequeue(m)) {
			Buffer.Enqueue(m);
			outArray.Add(m);
		}
	}
	else {
		while (Buffer.Dequeue(m)) {
			BufferAlt.Enqueue(m);
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