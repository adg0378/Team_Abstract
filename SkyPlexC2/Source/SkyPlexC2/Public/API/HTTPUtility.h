#pragma once

#include "CoreMinimal.h"
#include "AsynJSONUtility.h"
#include "HttpModule.h"
#include "Async/Async.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

// Forward declarations
class USPLogger;
typedef void CURL;

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

/*
 * HTTP request builder
 */
class SKYPLEXC2_API HTTPUtility
{
public:
	HTTPUtility();
	~HTTPUtility();

	HTTPUtility& SetUrl(const FString& url);

	HTTPUtility& SetPostFields(const FString& fields);

	HTTPUtility& SetPostBody(const TSharedPtr<FJsonObject>& JsonObject);

	HTTPUtility& AppendHeaders(TMap<FString, FString>& headers);

	HTTPUtility& AppendHeader(const FString& key, const FString& val);

	TFuture<FHTTPJSONResult<FString>> RequestString(HTTPRequestType RequestType);

	template <typename Schema>
	TFuture<FHTTPJSONResult<Schema>> RequestJSON(HTTPRequestType requestType) {
		TSharedRef<IHttpRequest> request = httpModule->CreateRequest();
		InitRequest(request, requestType);

		TSharedPtr<TPromise<FHTTPJSONResult<Schema>>> promise = MakeShared<TPromise<FHTTPJSONResult<Schema>>>();

		request->OnProcessRequestComplete().BindLambda(
			[promise](FHttpRequestPtr req, FHttpResponsePtr resp, bool success) {
				if (success && resp.IsValid()) {
					AsynJSONUtility::Parse<Schema>(resp->GetContentAsString())
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
