// Copyright (c) 2025 Synetos Aerospace

#pragma once

#include "CoreMinimal.h"
#include "State/SPManager.h"
#include "Delegates/DelegateCombinations.h"
#include "SPAuthManager.generated.h"

UENUM(BlueprintType)
enum class EAuthStep : uint8
{
	LOGGED_OUT UMETA(DisplayName = "Logged Out"),

	/** Verification email sent for the first time upon registering */
	SENT_EMAIL_VERIFICATION UMETA(DisplayName = "Sent email verificaiton"),

	/** User logged in after registering and still has not verified (present with a button to resend email then swap to sent_email_verification) */
	AWAITING_EMAIL_VERIFICATION UMETA(DisplayName = "Awaiting Email Verification"),
	AUTHENTICATED UMETA(DisplayName = "Authenticated"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAuthErrorDelegate, FString, ErrorMessage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAuthStepDelegate, EAuthStep, AuthStep);

/**
 * Handles user authentication and authenticating http requests
 */
UCLASS(Blueprintable, BlueprintType)
class SKYPLEXC2_API USPAuthManager : public USPManager
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Auth")
	FAuthErrorDelegate OnAuthenticationError;

	UPROPERTY(BlueprintAssignable, Category = "Auth")
	FAuthStepDelegate OnAuthStepChanged;

	UFUNCTION(BlueprintCallable)
	void RegisterAccount(const FString& Email, const FString& Password);

	UFUNCTION(BlueprintCallable)
	void LogIn(const FString& Email, const FString& Password);

	UFUNCTION(BlueprintCallable)
	void LogOut();

	UFUNCTION(BlueprintCallable)
	FString GetEmail() const;

	UFUNCTION(BlueprintCallable)
	bool IsAuthenticated() const;

	UFUNCTION(BlueprintCallable)
	EAuthStep GetAuthStep() const;

private:
	void SendVerificationEmail();

	void EmmitAuthError(const FString& ErrorMessage) const;

	void UpdateAuthStep(EAuthStep AuthStep);

	bool IsPasswordValid(const FString& Password);

	TFuture<bool> CheckEmailVerifiedStatus();

	void StartCheckEmailVerifiedTimer();
	void StopCheckEmailVerifiedTimer();
	void CheckEmailVerifiedTimerTick();

	void RefreshIdToken();

	void StartRefreshTokenTimer();
	void StopRefreshTokenTimer();
	void RefreshTokenTimerTick();
	
	FString EmailAddr;
	FString Uid;
	FString IdToken;
	FString RefreshToken;
	bool EmailVerified = false;
	EAuthStep CurrentAuthStep = EAuthStep::LOGGED_OUT;

	UPROPERTY()
	FTimerHandle CheckEmailVerifiedTimerHandle;

	bool CheckEmailVerifiedTimerActive = false;
	int TimesEmailVerifiedChecked = 0;

	UPROPERTY()
	FTimerHandle RefreshTokenTimerHandle;
	bool RefreshTokenTimerActive = false;
};
