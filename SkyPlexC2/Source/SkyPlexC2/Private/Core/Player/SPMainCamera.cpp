// Copyright (c) 2025 Synetos Aerospace

// Must add this if importing Cesium3DTileset.h
#define NOMINMAX
#include "Core/Player/SPMainCamera.h"
#include "Camera/CameraComponent.h"
#include "Core/Player/CesiumSpringArmComponent.h"
#include "CesiumFlyToComponent.h"
#include "CesiumOriginShiftComponent.h"
#include "Kismet/GameplayStatics.h"
#include "JsonObjectConverter.h"
#include "Util/FileUtility.h"
#include "Engine/CollisionProfile.h"
#include "Util/SPGeoUtility.h"
#include "Util/SPAssetUtility.h"
#include "Core/State/SPGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SPEnvConstants.h"
#include "CesiumGlobeAnchorComponent.h"
#include "Cesium3DTileset.h"

ASPMainCamera* ASPMainCamera::GetSPMainCamera(const UObject* WorldContextObject) {
	UWorld* World = WorldContextObject->GetWorld();
	APlayerController* PC = World ? UGameplayStatics::GetPlayerController(WorldContextObject, 0) : nullptr;
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	ASPMainCamera* MainCamera = Pawn ? Cast<ASPMainCamera>(Pawn) : nullptr;
		
	if (!MainCamera) {
		UE_LOG(LogTemp, Warning, TEXT("Failed to get SPMainCamera"));
	}
	return MainCamera;
}


ASPMainCamera::ASPMainCamera()
{
	CubeComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cube"));
	CubeComponent->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> CubeMaterial(TEXT("/Game/Materials/InteractableBlue.InteractableBlue"));
	if (CubeMesh.Succeeded()) {
		CubeComponent->SetStaticMesh(CubeMesh.Object);
		CubeComponent->SetWorldScale3D(FVector(3.0f));
		CubeComponent->SetMaterial(0, CubeMaterial.Object);
		CubeComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		CubeComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SpringArmComponent = CreateDefaultSubobject<UCesiumSpringArmComponent>(TEXT("CesiumSpringArm"));
	SpringArmComponent->SetupAttachment(CubeComponent);
	SpringArmComponent->TargetArmLength = 10000.0f;
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->bInheritPitch = false;
	SpringArmComponent->bInheritYaw = true;
	SpringArmComponent->bInheritRoll = true;
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->bEnableCameraRotationLag = false;
	SpringArmComponent->CameraLagSpeed = 4.0f;
	SpringArmComponent->CameraLagMaxDistance = 0.0f;
	SpringArmComponent->bDoCollisionTest = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->ProjectionMode = ECameraProjectionMode::Perspective;
	CameraComponent->FieldOfView = 90.0f;

	GlobeAnchorComponent = CreateDefaultSubobject<UCesiumGlobeAnchorComponent>(TEXT("GlobeAnchor"));
	GlobeAnchorComponent->SetAdjustOrientationForGlobeWhenMoving(true);
	GlobeAnchorComponent->SetTeleportWhenUpdatingTransform(true);
	GlobeAnchorComponent->bAutoActivate = true;

	FlyToComponent = CreateDefaultSubobject<UCesiumFlyToComponent>(TEXT("FlyTo"));
	FlyToComponent->bAutoActivate = true;

	OriginShiftComponent = CreateDefaultSubobject<UCesiumOriginShiftComponent>(TEXT("OriginShift"));
	OriginShiftComponent->SetMode(ECesiumOriginShiftMode::ChangeCesiumGeoreference);
	OriginShiftComponent->SetDistance(0.0);
	OriginShiftComponent->bAutoActivate = true;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxAcceleration = 1000000000.0f;
	Movement->BrakingFrictionFactor = 1000000.0f;
	Movement->DefaultLandMovementMode = EMovementMode::MOVE_Flying;
	Movement->DefaultWaterMovementMode = EMovementMode::MOVE_Flying;
	Movement->MaxFlySpeed = BaseMovementSpeed;
	Movement->BrakingDecelerationFlying = 1000000.0f;
}

void ASPMainCamera::BeginPlay()
{
	Super::BeginPlay();
	
	// TODO: set FOV from preferences and bind fov to on preferences updates

	LoadCameraLocationData();

	PerformZoomDependentUpdates();

	// TODO: set active camera in world manager once added
}

void ASPMainCamera::PostInitializeComponents() {
	Super::PostInitializeComponents();
	/*FlyToComponent->OnFlightComplete.AddDynamic(this, &ASPMainCamera::OnFlightCompleted);*/
}

void ASPMainCamera::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	SaveCameraLocationData();
	Super::EndPlay(EndPlayReason);
}

void ASPMainCamera::SetFOV(float FOVDegrees) {
	CameraComponent->SetFieldOfView(FOVDegrees);
}

void ASPMainCamera::CameraMovement(FVector Magnitude) {
	FVector ZWorldDirection;
	float ZScale;
	bool ZValid;
	CalculateZMovement(Magnitude.Z, ZWorldDirection, ZScale, ZValid);

	FRotator Rot = FRotator(0.0, GetControlRotation().Yaw, 0.0);

	// forward vector
	AddMovementInput(Rot.Vector(), Magnitude.X * MovementSpeed);

	FVector RightVector = FRotationMatrix(Rot).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightVector, Magnitude.Y * MovementSpeed);

	AddMovementInput(ZWorldDirection, ZScale);
}

void ASPMainCamera::CameraZoom(float Magnitude) {
	Zoom = FMath::Clamp(ZoomSpeed * Magnitude + Zoom, MinZoom, MaxZoom);
	PerformZoomDependentUpdates();
}

void ASPMainCamera::CameraOrbit(float Magnitude) {
	AddControllerYawInput(Magnitude * OrbitSpeed);
}

void ASPMainCamera::CameraPitch(float Magnitude) {
	if (CameraType != ECameraType::TopDown) {
		FRotator WorldRotation = SpringArmComponent->GetComponentTransform().GetRotation().Rotator();
		float NewPitch = FMath::ClampAngle(Magnitude * OrbitSpeed + WorldRotation.Pitch, MinPitch, MaxPitch);
		FRotator Rot = FRotator(NewPitch, WorldRotation.Yaw, WorldRotation.Roll);
		SpringArmComponent->SetWorldRotation(Rot);
	}
}

void ASPMainCamera::SetCameraTypeFromIndex(uint8 Index) {
	if (Index == 0U) {
		SetCameraType(ECameraType::FirstPerson);
	}
	else if (Index == 1U) {
		SetCameraType(ECameraType::TopDown);
	}
	else if (Index == 2U) {
		SetCameraType(ECameraType::Isometric);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Got invalid camera type while setting from index"));
	}
}

void ASPMainCamera::SetCameraType(ECameraType Type) {
	if (CameraType == Type) {
		return;
	}

	switch (Type) {
		case ECameraType::FirstPerson:
			SetFirstPersonCamera();
			break;
		case ECameraType::Isometric:
			SetIsometricCamera();
			break;
		case ECameraType::TopDown:
			SetTopDownCamera();
			break;
		default:
			UE_LOG(LogTemp, Warning, TEXT("Got invalid camera type while setting"));
	}

	CameraType = Type;
	PerformZoomDependentUpdates();
}

ECameraType ASPMainCamera::GetCameraType() const {
	return CameraType;
}

void ASPMainCamera::FlyTo(float Lon, float Lat) {
	FVector Vec = FVector(Lon, Lat, 0.0);

	FCesiumSampleHeightMostDetailedCallback CesiumCallback = FCesiumSampleHeightMostDetailedCallback::CreateLambda(
		[this](ACesium3DTileset* Tileset, const TArray<FCesiumSampleHeightResult>& Results, const TArray<FString>& Warnings) {
			for (const auto& Warning : Warnings) {
				UE_LOG(LogTemp, Warning, TEXT("%s"), *Warning);
			}

			if (Results.Num() > 0) {
				FlyToHeight(Results[0].LongitudeLatitudeHeight);
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("Recieved no height sample results for fly to"));
			}
		}
	);

	ASPGameState::GetSPGameState(this)->CesiumTileset->SampleHeightMostDetailed(TArray{ Vec }, CesiumCallback);
}

void ASPMainCamera::FlyToHeight(FVector LonLatHeight) {
	LonLatHeight.Z += DistanceAboveEarthCM / 100.0f;

	FlyToComponent->InterruptFlight();

	// TODO: get from preferences manager once added
	bool InstantFlyTo = false;

	if (InstantFlyTo) {
		// TODO: update timezone when world manager added
		InstantTeleport(LonLatHeight);
	}
	else {
		float Distance = USPGeoUtility::GeodesicDistance(LonLatHeight, GlobeAnchorComponent->GetLongitudeLatitudeHeight());
		FlyToComponent->Duration = FMath::Min(Distance / MaxFlyToSpeed, MaxFlyToTimeS);

		// TODO: update timezone when world manager added
		FlyToComponent->FlyToLocationLongitudeLatitudeHeight(LonLatHeight, 0.0, 0.0, false);
	}
}

void ASPMainCamera::InstantTeleport(FVector LonLatHeight) {
	UWorld* World = GetWorld();
	World->GetTimerManager().ClearTimer(InstantTeleportDelayHandle);

	SpringArmComponent->bEnableCameraLag = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->DisableMovement();

	GlobeAnchorComponent->MoveToLongitudeLatitudeHeight(LonLatHeight);

	World->GetTimerManager().SetTimer(InstantTeleportDelayHandle, this, &ASPMainCamera::OnInstantTeleportDelayFinished, 0.25f);

	// TODO: cull objects when world manager added
}

void ASPMainCamera::OnInstantTeleportDelayFinished() {
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->SetMovementMode(EMovementMode::MOVE_Flying);
	SpringArmComponent->bEnableCameraLag = true;
}

void ASPMainCamera::CalculateZMovement(float ZMagnitude, FVector& WorldDirection, float& Scale, bool& Valid) {
	int TraceDistance = 1000000000;
	FVector WorldScale = CubeComponent->GetComponentTransform().GetScale3D();
	float HalfHeight = WorldScale.X * 50.0f;
	FVector UpVector = CubeComponent->GetUpVector();
	FVector WorldLocation = CubeComponent->GetComponentLocation();

	WorldDirection = UpVector;
	Valid = true;

	UWorld* World = GetWorld();
	
	FHitResult Result;
	FVector DownTraceEnd = WorldLocation + (UpVector * TraceDistance * -1.0);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	DrawDebugLine(World, WorldLocation, DownTraceEnd, FColor::Green, false, 5.0f, 0, 2.0f);
	if (World->LineTraceSingleByChannel(Result, WorldLocation, DownTraceEnd, USPAssetUtility::LandscapeTraceChannel, Params)) {
		float DistanceToMoveDown = Result.Distance - DistanceAboveEarthCM - HalfHeight;

		if (CameraType == ECameraType::FirstPerson && DistanceToMoveDown > 0.0f) {
			Scale = ZMagnitude * MovementSpeed;
		}
		else {
			UCharacterMovementComponent* Movement = GetCharacterMovement();
			float Dt = DistanceToMoveDown / World->GetDeltaSeconds();
			float DtPerFlySpeed = FMath::IsNearlyZero(Movement->MaxFlySpeed) ? 0.0f : Dt / Movement->MaxFlySpeed;
			Scale = -DtPerFlySpeed;
		}
	}
	else {
		FVector UpStart = WorldLocation + UpVector * TraceDistance;
		FVector UpEnd = WorldLocation - UpVector * TraceDistance;
		DrawDebugLine(World, UpStart, UpEnd, FColor::Green, false, 5.0f, 0, 2.0f);
		if (World->LineTraceSingleByChannel(Result, UpStart, UpEnd, ECC_Visibility, Params)) {
			UCharacterMovementComponent* Movement = GetCharacterMovement();
			Scale = ((TraceDistance - Result.Distance) + HalfHeight + DistanceAboveEarthCM) / World->GetDeltaSeconds() / Movement->MaxFlySpeed;
		}
		else {
			Valid = CameraType == ECameraType::FirstPerson;
			Scale = ZMagnitude * MovementSpeed;
		}
	}
}

void ASPMainCamera::SaveCameraLocationData() {
	FVector LonLatHeight = GlobeAnchorComponent->GetLongitudeLatitudeHeight();
	FRotator WorldRotation = SpringArmComponent->GetComponentTransform().GetRotation().Rotator();
	float TargetArmLength = SpringArmComponent->TargetArmLength;

	FCameraLocationData Data{
		.Lon = LonLatHeight.X,
		.Lat = LonLatHeight.Y,
		.Height = LonLatHeight.Z,
		.ArmLength = SpringArmComponent->TargetArmLength,
		.Roll = WorldRotation.Roll,
		.Pitch = WorldRotation.Pitch,
		.Yaw = WorldRotation.Yaw,
	};

	FString JsonString;

	if (!FJsonObjectConverter::UStructToJsonObjectString<FCameraLocationData>(Data, JsonString)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to stringify camera location data"));
		return;
	}

	if (!UFileUtility::WriteStringToFile(*USPEnvConstants::GetCameraLocationPath(), *JsonString)) {
		UE_LOG(LogTemp, Error, TEXT("Failed to save camera location data"));
	}
}

void ASPMainCamera::LoadCameraLocationData() {
	bool Failed = false;
	FCameraLocationData ParsedLocation;

	if (FPaths::FileExists(USPEnvConstants::GetCameraLocationPath())) {
		FString FileContents;

		if (!FFileHelper::LoadFileToString(FileContents, *USPEnvConstants::GetCameraLocationPath())) {
			UE_LOG(LogTemp, Error, TEXT("Failed to load camera location data"));
			Failed = true;
		} else if (!FJsonObjectConverter::JsonObjectStringToUStruct<FCameraLocationData>(FileContents, &ParsedLocation, 0, 0)) {
			UE_LOG(LogTemp, Error, TEXT("Failed to parse camera location data"));
			Failed = true;
		}
		else {
		}
	}
	else {
		if (!UFileUtility::WriteStringToFile(*USPEnvConstants::GetCameraLocationPath(), TEXT(""))) {
			UE_LOG(LogTemp, Error, TEXT("Failed to create camera location file"));
		}
		Failed = true;
	}

	FVector StartLoc = Failed ? DefaultStartLocation : FVector(ParsedLocation.Lon, ParsedLocation.Lat, ParsedLocation.Height);
	GlobeAnchorComponent->MoveToLongitudeLatitudeHeight(StartLoc);

	if (!Failed) {
		Zoom = ParsedLocation.ArmLength;
	}

	// TODO: Update time zone once world manager is added

	// This correctly resets the object's rotation to avoid strange angle issues on startup
	FlyToHeight(StartLoc);

	if (!Failed) {
		SpringArmComponent->SetWorldRotation(FRotator(ParsedLocation.Pitch, ParsedLocation.Yaw, ParsedLocation.Roll));
	}
}

void ASPMainCamera::PerformZoomDependentUpdates() {
	float IncreaseMultiplier = Zoom / MinZoom;

	if (CameraType != ECameraType::FirstPerson) {
		SpringArmComponent->TargetArmLength = Zoom;
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxFlySpeed = BaseMovementSpeed * (IncreaseMultiplier / 2);

	ZoomSpeed = FMath::Sqrt(IncreaseMultiplier) * BaseZoomSpeed;
}

void ASPMainCamera::SetIsometricCamera() {
	Zoom = IsoCamProps.ArmLength;

	MaxZoom = DefaultMaxZoom;

	FRotator WorldRotation = SpringArmComponent->GetComponentTransform().GetRotation().Rotator();
	FRotator Rot = FRotator(IsoCamProps.Pitch, WorldRotation.Yaw, WorldRotation.Roll);
	SpringArmComponent->SetWorldRotation(Rot);

	MaxPitch = DefaultMaxPitch;
}

void ASPMainCamera::SetTopDownCamera() {
	FRotator WorldRotation = SpringArmComponent->GetComponentTransform().GetRotation().Rotator();

	if (CameraType == ECameraType::Isometric) {
		IsoCamProps = FIsometricCameraProps{ .Pitch = WorldRotation.Pitch, .ArmLength = Zoom };
	}

	MaxZoom = DefaultMaxZoom;

	FRotator Rot = FRotator(MinPitch, WorldRotation.Yaw, WorldRotation.Roll);
	SpringArmComponent->SetWorldRotation(Rot);

	MaxPitch = DefaultMaxPitch;
}

void ASPMainCamera::SetFirstPersonCamera() {

	if (CameraType == ECameraType::Isometric) {
		FRotator WorldRotation = SpringArmComponent->GetComponentTransform().GetRotation().Rotator();
		IsoCamProps = FIsometricCameraProps{ .Pitch = WorldRotation.Pitch, .ArmLength = SpringArmComponent->TargetArmLength };
	}

	SpringArmComponent->TargetArmLength = MinZoom;
	MaxZoom = FirstPersonMaxZoom;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxFlySpeed *= 10.0;

	MaxPitch = FirstPersonMaxPitch;
}

void ASPMainCamera::OnFlightCompleted() {
	// TODO: cull objects once world manager is added
}
