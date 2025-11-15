// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Player/SPPlayerController.h"
#include "GameFramework/GameUserSettings.h"
#include "Core/Player/SPMainCamera.h"

ASPPlayerController::ASPPlayerController() {
	bShowMouseCursor = true;
}

void ASPPlayerController::BeginPlay() {
	Super::BeginPlay();

	MainCameraRef = ASPMainCamera::GetSPMainCamera(this);

	SetupEnhancedInput();
	SetShowMouseCursor(true);

	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (Settings) {
		Settings->SetFullscreenMode(EWindowMode::Windowed);
	}
}

void ASPPlayerController::IACameraPitch(float ActionValue) {
	if (MainCameraRef) {
		MainCameraRef->CameraPitch(ActionValue);
	}
}

void ASPPlayerController::IAFreeCamera() {
	FreeCameraPressed = !FreeCameraPressed;
}

void ASPPlayerController::IAShowCursorInFirstPerson() {
	if (MainCameraRef && MainCameraRef->GetCameraType() == ECameraType::FirstPerson) {
		ShowCursorInFirstPersonPressed = !ShowCursorInFirstPersonPressed;

		SetShowMouseCursor(ShowCursorInFirstPersonPressed);
	}
}

void ASPPlayerController::IAMouseX(float ActionValue) {
	if (MainCameraRef && ((MainCameraRef->GetCameraType() == ECameraType::FirstPerson && !ShowCursorInFirstPersonPressed) || FreeCameraPressed)) {
		MainCameraRef->CameraOrbit(ActionValue);
	}
}

void ASPPlayerController::IAMouseY(float ActionValue) {
	if (MainCameraRef && ((MainCameraRef->GetCameraType() == ECameraType::FirstPerson && !ShowCursorInFirstPersonPressed) || FreeCameraPressed)) {
		MainCameraRef->CameraPitch(ActionValue);
	}
}

void ASPPlayerController::IACameraZoomIn() {
	if (MainCameraRef) {
		MainCameraRef->CameraZoom(-1.0f);
	}
}

void ASPPlayerController::IACameraZoomOut() {
	if (MainCameraRef) {
		MainCameraRef->CameraZoom(1.0f);
	}
}

void ASPPlayerController::IACameraOrbit(float ActionValue) {
	if (MainCameraRef) {
		MainCameraRef->CameraOrbit(ActionValue);
	}
}

void ASPPlayerController::IAMovement(FVector ActionValue) {
	if (MainCameraRef) {
		MainCameraRef->CameraMovement(ActionValue);
	}
}

void ASPPlayerController::ChangeCameraType(uint8 CameraTypeIndex) {
	if (CameraTypeIndex == 0U) {
		IASwapCamera1();
	}
	else if (CameraTypeIndex == 1U) {
		IASwapCamera2();
	}
	else if (CameraTypeIndex == 2U) {
		IASwapCamera3();
	}
}

void ASPPlayerController::IASwapCamera1() {
	if (MainCameraRef) {
		SetShowMouseCursor(false);
		ShowCursorInFirstPersonPressed = false;

		MainCameraRef->SetCameraTypeFromIndex(0U);

		// TODO: make a camera swap event that the WB compass widget can subscribe to
		// TODO: also swap MainCameraRef to getting the active camera instead and swapping to a specific camera index
	}
}

void ASPPlayerController::IASwapCamera2() {
	if (MainCameraRef) {
		SetShowMouseCursor(true);

		MainCameraRef->SetCameraTypeFromIndex(1U);
	}
}

void ASPPlayerController::IASwapCamera3() {
	if (MainCameraRef) {
		SetShowMouseCursor(true);

		MainCameraRef->SetCameraTypeFromIndex(2U);
	}
}

void ASPPlayerController::SetupEnhancedInput_Implementation() {}