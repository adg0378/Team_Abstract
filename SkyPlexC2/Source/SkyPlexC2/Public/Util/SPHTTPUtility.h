// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "UObject/NoExportTypes.h"
#include "SPHTTPUtility.generated.h"

template <typename T>
struct FJSONParseResult {
	bool didFail;
	T result;
};

struct FJSONStringifyResult {
	bool didFail;
	FString result;
};

enum class HTTPRequestType : uint8 {
	GET = 0,
	POST,
};

struct FHTTPRequestBuilderData {
public:
	FString url;
	TMap<FString, FString> headers;
	FString postFields;
};

template <typename Schema>
struct FHTTPJSONResult {
	bool didFail;
	FString errMsg;
	Schema response;
};

struct FHTTPResult {
	bool didFail;
	FString errMsg;
	FString response;
};

/**
 * HTTP request builder
 */
UCLASS()
class SKYPLEXC2_API USPHTTPUtility : public UObject
{
	GENERATED_BODY()
	
public:
	USPHTTPUtility();
	~USPHTTPUtility();

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

	USPHTTPUtility& SetUrl(const FString& url);

	USPHTTPUtility& SetPostFields(const FString& fields);

	USPHTTPUtility& SetPostBody(const TSharedPtr<FJsonObject>& JsonObject);

	USPHTTPUtility& AppendHeaders(TMap<FString, FString>& headers);

	USPHTTPUtility& AppendHeader(const FString& key, const FString& val);

	TFuture<FHTTPJSONResult<FString>> RequestString(HTTPRequestType RequestType);

	template <typename Schema>
	TFuture<FHTTPJSONResult<Schema>> RequestJSON(HTTPRequestType requestType) {
		TSharedRef<IHttpRequest> request = httpModule->CreateRequest();
		InitRequest(request, requestType);

		TSharedPtr<TPromise<FHTTPJSONResult<Schema>>> promise = MakeShared<TPromise<FHTTPJSONResult<Schema>>>();

		request->OnProcessRequestComplete().BindLambda(
			[promise](FHttpRequestPtr req, FHttpResponsePtr resp, bool success) {
				if (success && resp.IsValid()) {
					Parse<Schema>(resp->GetContentAsString())
						.Then([promise](TFuture<FJSONParseResult<Schema>> resFut) {
						const FJSONParseResult<Schema>& jsonRes = resFut.Get();
						FHTTPJSONResult<Schema> result;
						if (jsonRes.didFail) {
							result.didFail = true;
							result.errMsg = TEXT("Error parsing HTTP result");
						}
						else {
							result.didFail = false;
							result.response = jsonRes.result;
						}
						promise->SetValue(result);
							});
				}
				else {
					FHTTPJSONResult<Schema> result;
					result.didFail = true;
					result.errMsg = TEXT("Error completing HTTP request");
					promise->SetValue(result);
				}
			});

		request->ProcessRequest();
		return promise->GetFuture();
	}

	TFuture<FHTTPResult> RequestFile(HTTPRequestType requestType, const FString& filePath);

	/*void RequestFile(HTTPRequestType requestType, const FString& filePath);*/

private:
	void InitRequest(TSharedRef<IHttpRequest> request, HTTPRequestType requestType);

	FHttpModule* httpModule;

	FHTTPRequestBuilderData requestData;
};
