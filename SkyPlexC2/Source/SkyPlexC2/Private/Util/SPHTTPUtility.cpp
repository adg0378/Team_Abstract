// Copyright (c) 2025 Synetos Aerospace


#include "Util/SPHTTPUtility.h"

USPHTTPUtility::USPHTTPUtility() {
    httpModule = &FHttpModule::Get();
    requestData = FHTTPRequestBuilderData{};
}

USPHTTPUtility::~USPHTTPUtility() {
    httpModule = nullptr;
}

void USPHTTPUtility::InitRequest(TSharedRef<IHttpRequest> request, HTTPRequestType requestType) {
    request->SetURL(requestData.url);

    switch (requestType) {
    case HTTPRequestType::GET:
        request->SetVerb("GET");
        break;
    case HTTPRequestType::POST:
        request->SetVerb("POST");
        request->SetContentAsString(requestData.postFields);
        break;
    default:
        break;
    }

    for (const TPair<FString, FString>& header : requestData.headers) {
        request->SetHeader(header.Key, header.Value);
    }
}

USPHTTPUtility& USPHTTPUtility::SetUrl(const FString& url) {
    requestData.url = url;
    return *this;
}

USPHTTPUtility& USPHTTPUtility::SetPostFields(const FString& fields) {
    requestData.postFields = fields;
    return *this;
}

USPHTTPUtility& USPHTTPUtility::SetPostBody(const TSharedPtr<FJsonObject>& JsonObject) {
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    requestData.postFields = RequestBody;
    return *this;
}

USPHTTPUtility& USPHTTPUtility::AppendHeaders(TMap<FString, FString>& headers) {
    requestData.headers.Append(headers);
    return *this;
}

USPHTTPUtility& USPHTTPUtility::AppendHeader(const FString& key, const FString& val) {
    requestData.headers.Add(key, val);
    return *this;
}

TFuture<FHTTPJSONResult<FString>> USPHTTPUtility::RequestString(HTTPRequestType RequestType) {
    TSharedRef<IHttpRequest> Request = httpModule->CreateRequest();
    InitRequest(Request, RequestType);

    TSharedPtr<TPromise<FHTTPJSONResult<FString>>> Promise = MakeShared<TPromise<FHTTPJSONResult<FString>>>();

    Request->OnProcessRequestComplete().BindLambda(
        [Promise](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool Success) {
            if (Success && Resp.IsValid()) {
                FHTTPJSONResult<FString> Result;
                Result.didFail = false;
                Result.response = Resp->GetContentAsString();
                Promise->SetValue(Result);
            }
            else {
                FHTTPJSONResult<FString> result;
                result.didFail = true;
                result.errMsg = TEXT("Error completing HTTP request");
                Promise->SetValue(result);
            }
        });

    Request->ProcessRequest();
    return Promise->GetFuture();
}

TFuture<FHTTPResult> USPHTTPUtility::RequestFile(HTTPRequestType requestType, const FString& filePath) {
    TSharedRef<IHttpRequest> request = httpModule->CreateRequest();
    InitRequest(request, requestType);

    TSharedPtr<TPromise<FHTTPResult>> promise = MakeShared<TPromise<FHTTPResult>>();

    request->OnProcessRequestComplete().BindLambda(
        [promise, filePath](FHttpRequestPtr req, FHttpResponsePtr resp, bool success) {
            FHTTPResult result;
            if (success && resp.IsValid()) {
                const TArray<uint8>& content = resp->GetContent();
                if (FFileHelper::SaveArrayToFile(content, *filePath)) {
                    result.didFail = false;
                    result.response = filePath;
                }
                else {
                    result.didFail = true;
                    result.errMsg = FString::Printf(TEXT("Error writing to %s"), *filePath);
                }
            }
            else {
                result.didFail = true;
                result.errMsg = TEXT("Error completing HTTP request");
            }
            promise->SetValue(result);
        });

    request->ProcessRequest();
    return promise->GetFuture();
}