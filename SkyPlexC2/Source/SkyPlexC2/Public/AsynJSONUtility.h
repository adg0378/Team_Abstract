// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "Async/Future.h"
#include "Async/Async.h"

template <typename T>
struct FJSONParseResult {
	bool didFail;
	T result;
};

struct FJSONStringifyResult {
	bool didFail;
	FString result;
};

/**
 * Exposes an asynchronous json parsing method
 */
class SKYPLEXC2_API AsynJSONUtility
{
public:
	template <typename Schema>
	static TFuture<FJSONParseResult<Schema>> Parse(const FString& jsonString) {
		TSharedRef<TPromise<FJSONParseResult<Schema>>> promise = MakeShared<TPromise<FJSONParseResult<Schema>>>();

		FString localJSON = jsonString;
		Async(EAsyncExecution::ThreadPool, [promise, localJSON]() {
			Schema parsedData;
			bool success = FJsonObjectConverter::JsonObjectStringToUStruct(localJSON, &parsedData, 0, 0);
			FJSONParseResult<Schema> result;
			result.didFail = !success;
			result.result = parsedData;
			promise->SetValue(result);
		});

		return promise->GetFuture();
	}

	template <typename Schema>
	static TFuture<FJSONStringifyResult> Stringify(const Schema& jsonStruct) {
		TSharedRef<TPromise<FJSONStringifyResult>> promise = MakeShared<TPromise<FJSONStringifyResult>>();

		Schema localJSONStruct = jsonStruct;
		Async(EAsyncExecution::ThreadPool, [promise, localJSONStruct]() {
			FJSONStringifyResult result;
			result.didFail = !FJsonObjectConverter::UStructToJsonObjectString<Schema>(localJSONStruct, result.result);
			promise->SetValue(result);
		});
		return promise->GetFuture();
	}
};
