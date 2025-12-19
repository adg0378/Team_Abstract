// Copyright (c) 2025 Synetos Aerospace


#include "State/SPAuthManager.h"
#include "SPEnvConstants.h"
#include "API/HTTPUtility.h"
#include <regex>

const FString FIREBASE_API_KEY = TEXT("AIzaSyAUMPdWFA_vfVDrASztuAK7trtak-9ddu8");

FString USPAuthManager::GetEmail() const {
	return EmailAddr;
}

bool USPAuthManager::IsAuthenticated() const {
	return CurrentAuthStep == EAuthStep::AUTHENTICATED;
}

EAuthStep USPAuthManager::GetAuthStep() const {
	return CurrentAuthStep;
}

void USPAuthManager::EmmitAuthError(const FString& ErrorMessage) const {
	UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
	if (IsInGameThread()) {
		OnAuthenticationError.Broadcast(ErrorMessage);
	}
	else {
		AsyncTask(ENamedThreads::GameThread, [this, ErrorMessage]() {
			OnAuthenticationError.Broadcast(ErrorMessage);
			});
	}
}

void USPAuthManager::UpdateAuthStep(EAuthStep AuthStep) {
	CurrentAuthStep = AuthStep;
	if (IsInGameThread()) {
		OnAuthStepChanged.Broadcast(CurrentAuthStep);
	}
	else {
		AsyncTask(ENamedThreads::GameThread, [this]() {
			OnAuthStepChanged.Broadcast(CurrentAuthStep);
			});
	}
}

bool USPAuthManager::IsPasswordValid(const FString& Password) {
	std::regex Pattern("^(?=.*[A-Z])(?=.*[a-z])(?=.*[^A-Za-z0-9]).{8,}$");
	bool IsValid = std::regex_match(TCHAR_TO_UTF8(*Password), Pattern);
	if (!IsValid) {
		EmmitAuthError(TEXT("Passwords must be at least 8 characters long, contain a number, capital letter, and special character"));
	}
	return IsValid;
}

void USPAuthManager::LogIn(const FString& Email, const FString& Password) {
	if (!IsPasswordValid(Password)) {
		return;
	}

	FString Url = FString::Printf(TEXT("%saccounts:signInWithPassword?key=%s"), *USPEnvConstants::FIREBASE_URL, *FIREBASE_API_KEY);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("email", Email);
	JsonObject->SetStringField("password", Password);
	JsonObject->SetBoolField("returnSecureToken", true);

	EmailAddr = Email;

	HTTPUtility Client;
	Client.SetUrl(Url)
		.AppendHeader("Content-Type", "application/json")
		.SetPostBody(JsonObject)
		.RequestString(HTTPRequestType::POST)
		.Then([this](TFuture<FHTTPJSONResult<FString>> ResFut) {
			const FHTTPJSONResult<FString>& Result = ResFut.Get();

			TSharedPtr<FJsonObject> JsonResponse;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Result.response);

			if (!Result.didFail && FJsonSerializer::Deserialize(Reader, JsonResponse) && JsonResponse.IsValid()) {
				if (JsonResponse->HasField(TEXT("localId"))) {
					Uid = JsonResponse->GetStringField(TEXT("localId"));
					IdToken = JsonResponse->GetStringField(TEXT("idToken"));
					RefreshToken = JsonResponse->GetStringField(TEXT("refreshToken"));
					
					if (JsonResponse->HasField(TEXT("emailVerified")) && !JsonResponse->HasTypedField<EJson::Null>(TEXT("emailVerified"))) {
						EmailVerified = JsonResponse->GetBoolField(TEXT("emailVerified"));
						if (EmailVerified) {
							UpdateAuthStep(EAuthStep::AUTHENTICATED);
							StartRefreshTokenTimer();
						}
						else {
							SendVerificationEmail();
						}
					}
					else {
						CheckEmailVerifiedStatus()
							.Then([this](TFuture<bool> ResFut) {
								EmailVerified = ResFut.Get();
								if (EmailVerified) {
									UpdateAuthStep(EAuthStep::AUTHENTICATED);
									StartRefreshTokenTimer();
								}
								else {
									SendVerificationEmail();
								}
							});
					}
				}
				else if (JsonResponse->HasField(TEXT("error"))) {
					FString ErrorMsg = JsonResponse->GetObjectField(TEXT("error"))->GetStringField(TEXT("message"));
					EmmitAuthError(FString::Printf(TEXT("Error logging in: %s"), *ErrorMsg));
				}
			}
			else {
				EmmitAuthError(TEXT("Encountered unknown error while logging in."));
			}
		});
}

void USPAuthManager::LogOut() {
	StopCheckEmailVerifiedTimer();
	StopRefreshTokenTimer();
	UpdateAuthStep(EAuthStep::LOGGED_OUT);
	EmailVerified = false;
	RefreshToken = Uid = IdToken = EmailAddr = "";
}

void USPAuthManager::RegisterAccount(const FString& Email, const FString& Password) {
	if (!IsPasswordValid(Password)) {
		return;
	}

	FString Url = FString::Printf(TEXT("%saccounts:signUp?key=%s"), *USPEnvConstants::FIREBASE_URL, *FIREBASE_API_KEY);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("email", Email);
	JsonObject->SetStringField("password", Password);
	JsonObject->SetBoolField("returnSecureToken", true);

	EmailAddr = Email;

	HTTPUtility Client;
	Client.SetUrl(Url)
		.AppendHeader("Content-Type", "application/json")
		.SetPostBody(JsonObject)
		.RequestString(HTTPRequestType::POST)
		.Then([this](TFuture<FHTTPJSONResult<FString>> ResFut) {
			const FHTTPJSONResult<FString>& Result = ResFut.Get();

			TSharedPtr<FJsonObject> JsonResponse;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Result.response);

			if (!Result.didFail && FJsonSerializer::Deserialize(Reader, JsonResponse) && JsonResponse.IsValid()) {
				if (JsonResponse->HasField(TEXT("localId"))) {
					Uid = JsonResponse->GetStringField(TEXT("localId"));
					IdToken = JsonResponse->GetStringField(TEXT("idToken"));
					RefreshToken = JsonResponse->GetStringField(TEXT("refreshToken"));
					EmailVerified = false;

					SendVerificationEmail();
				}
				else if (JsonResponse->HasField(TEXT("error"))) {
					FString ErrorMsg = JsonResponse->GetObjectField(TEXT("error"))->GetStringField(TEXT("message"));
					EmmitAuthError(FString::Printf(TEXT("Error registering new account: %s"), *ErrorMsg));
				}
				else {
					EmmitAuthError(TEXT("Unknown error registering new account"));
				}
			}
			else {
				EmmitAuthError(TEXT("Unknown error registering new account"));
			}
			});
}

void USPAuthManager::SendVerificationEmail() {
	FString Url = FString::Printf(TEXT("%saccounts:sendOobCode?key=%s"), *USPEnvConstants::FIREBASE_URL, *FIREBASE_API_KEY);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("requestType", "VERIFY_EMAIL");
	JsonObject->SetStringField("idToken", IdToken);
	UE_LOG(LogTemp, Warning, TEXT("Sending verification email"))

	UpdateAuthStep(EAuthStep::SENT_EMAIL_VERIFICATION);

	HTTPUtility Client;
	Client.SetUrl(Url)
		.AppendHeader("Content-Type", "application/json")
		.SetPostBody(JsonObject)
		.RequestString(HTTPRequestType::POST)
		.Then([this](TFuture<FHTTPJSONResult<FString>> ResFut) {
			const FHTTPJSONResult<FString>& Result = ResFut.Get();
			UE_LOG(LogTemp, Warning, TEXT("Got a response from verification email: %s"), *Result.response);
			if (Result.didFail) {
				EmmitAuthError(TEXT("Failed to send verification email. Please try again later."));
			}
			else {
				StartCheckEmailVerifiedTimer();
			}
			});
}

void USPAuthManager::RefreshIdToken() {
	FString Url = FString::Printf(TEXT("%stoken?key=%s"), *USPEnvConstants::GOOGLE_TOKEN_URL, *FIREBASE_API_KEY);

	FString RequestBody = FString::Printf(TEXT("grant_type=refresh_token&refresh_token=%s"), *RefreshToken);

	HTTPUtility Client;
	Client.SetUrl(Url)
		.AppendHeader("Content-Type", "application/x-www-form-urlencoded")
		.SetPostFields(RequestBody)
		.RequestString(HTTPRequestType::POST)
		.Then([this](TFuture<FHTTPJSONResult<FString>> ResFut) {
			const FHTTPJSONResult<FString>& Result = ResFut.Get();

			TSharedPtr<FJsonObject> JsonResponse;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Result.response);

			if (!Result.didFail && FJsonSerializer::Deserialize(Reader, JsonResponse) && JsonResponse.IsValid()) {
				if (JsonResponse->HasField(TEXT("id_token"))) {
					IdToken = JsonResponse->GetStringField(TEXT("id_token"));
					RefreshToken = JsonResponse->GetStringField(TEXT("refresh_token"));
				}
				else {
					LOG->Alert(TEXT("Encountered unknown error while attempting to re-authenticate."));
				}
			}
			else {
				LOG->Alert(TEXT("Failed to re-authenticate."));
			}

		});
}

void USPAuthManager::StartRefreshTokenTimer() {
	if (RefreshTokenTimerActive) {
		return;
	}
	FTimerManagerTimerParameters TimerParams;
	TimerParams.bLoop = TimerParams.bMaxOncePerFrame = true;
	RefreshTokenTimerActive = true;

	// refresh a few minutes less than every hour
	GetWorld()->GetTimerManager().SetTimer(RefreshTokenTimerHandle, this, &USPAuthManager::RefreshTokenTimerTick, 3450.0f, TimerParams);
}

void USPAuthManager::StopRefreshTokenTimer() {
	if (!CheckEmailVerifiedTimerActive) {
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(RefreshTokenTimerHandle);
	RefreshTokenTimerActive = false;
}

void USPAuthManager::RefreshTokenTimerTick() {
	if (!RefreshTokenTimerActive) {
		return;
	}
	RefreshIdToken();
}

void USPAuthManager::StartCheckEmailVerifiedTimer() {
	if (CheckEmailVerifiedTimerActive) {
		return;
	}
	FTimerManagerTimerParameters TimerParams;
	TimerParams.bLoop = TimerParams.bMaxOncePerFrame = true;
	CheckEmailVerifiedTimerActive = true;
	TimesEmailVerifiedChecked = 0;
	GetWorld()->GetTimerManager().SetTimer(CheckEmailVerifiedTimerHandle, this, &USPAuthManager::CheckEmailVerifiedTimerTick, 10.0f, TimerParams);
}

void USPAuthManager::StopCheckEmailVerifiedTimer() {
	if (!CheckEmailVerifiedTimerActive) {
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(CheckEmailVerifiedTimerHandle);
	CheckEmailVerifiedTimerActive = false;
}

void USPAuthManager::CheckEmailVerifiedTimerTick() {
	if (!CheckEmailVerifiedTimerActive) {
		return;
	}
	// only check for 6 minutes, then stop
	if (EmailVerified || TimesEmailVerifiedChecked >= 36) {
		StopCheckEmailVerifiedTimer();
		return;
	}
	CheckEmailVerifiedStatus()
		.Then([this](TFuture<bool> ResFut) {
			const bool& Result = ResFut.Get();
			if (Result) {
				EmailVerified = true;
				StopCheckEmailVerifiedTimer();
				UpdateAuthStep(EAuthStep::AUTHENTICATED);
			}
		});
}

TFuture<bool> USPAuthManager::CheckEmailVerifiedStatus() {
	FString Url = FString::Printf(TEXT("%saccounts:lookup?key=%s"), *USPEnvConstants::FIREBASE_URL, *FIREBASE_API_KEY);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField("idToken", IdToken);

	TSharedPtr<TPromise<bool>> Promise = MakeShared<TPromise<bool>>();

	HTTPUtility Client;
	Client.SetUrl(Url)
		.AppendHeader("Content-Type", "application/json")
		.SetPostBody(JsonObject)
		.RequestString(HTTPRequestType::POST)
		.Then([this, Promise](TFuture<FHTTPJSONResult<FString>> ResFut) {
			const FHTTPJSONResult<FString>& Result = ResFut.Get();
			if (Result.didFail) {
				EmmitAuthError(TEXT("Failed to check email verification status."));
				Promise->SetValue(false);
				return;
			}

			TSharedPtr<FJsonObject> JsonResponse;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Result.response);
			if (FJsonSerializer::Deserialize(Reader, JsonResponse) && JsonResponse.IsValid()) {

				const TArray<TSharedPtr<FJsonValue>>* UsersArray;
				if (JsonResponse->TryGetArrayField(TEXT("users"), UsersArray) && UsersArray->Num() > 0) {
					const TSharedPtr<FJsonObject> UserObj = (*UsersArray)[0]->AsObject();
					bool IsEmailVerified = false;
					if (UserObj->TryGetBoolField(TEXT("emailVerified"), IsEmailVerified)) {
						Promise->SetValue(IsEmailVerified);
						return;
					}
				}
			}
			EmmitAuthError(TEXT("Unknown error checking email verification status."));
			Promise->SetValue(false);
		});
	return Promise->GetFuture();
}