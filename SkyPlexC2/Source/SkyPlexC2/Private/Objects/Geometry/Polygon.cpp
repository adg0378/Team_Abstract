// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Geometry/Polygon.h"
#include "Objects/Geometry/PlaceablePoint.h"
#include "Player/SPPlayerController.h"
#include <CesiumGlobeAnchorComponent.h>
#include <CesiumGeoreference.h>
#include "CesiumGeospatial/Cartographic.h"
#include "SPUtility.h"
#include "State/SPGameState.h"

#undef GetObject

void APolygon::DestroySelf_Implementation() {
	if (LinkedProvider.GetObject() != nullptr) {
		IInteractionBoxProvider::Execute_DestroySelf(LinkedProvider.GetObject());
	}
	else {
		DestroyPolygon();
	}
}

void APolygon::DestroyPolygon() {
	if (IsClosed) {
		UnHighlight();
	}
	ClearAllPoints();
	Destroy();
}

void APolygon::ClearAllPoints() {
	Length = 0;
	if (!Head.IsValid()) {
		return;
	}

	TSharedPtr<FPolygonListNode> Current = Head;
	Head.Reset();

	do {
		TSharedPtr<FPolygonListNode> NextNode = Current->Next;

		Current->Point->Deselect();
		Current->Point->Destroy();
		Current->Next.Reset();
		Current->Prev.Reset();

		Current = NextNode;
	} while (Current.IsValid() && Current != Head);
}

void APolygon::CanBeClosed(bool& OutCanBeClosed) const {
	OutCanBeClosed = Length >= 3;
}

void APolygon::GetHead(APlaceablePoint*& OutPoint) const {
	OutPoint = Head->Point;
}

void APolygon::Highlight_Implementation() {}
void APolygon::UnHighlight_Implementation() {}
void APolygon::HighlightOnly_Implementation(bool IsHighlighted) {}
void APolygon::OutlineOnly_Implementation(bool IsOutlined) {}

void APolygon::CloseFromClickMe() {
	Head->Point->OnClickMeClick.RemoveDynamic(this, &APolygon::CloseFromClickMe);
	Close();
	ASPPlayerController* PC = USPUtility::GetSPPlayerController(this);
	PC->SpawnActor();
}

void APolygon::Close() {
	IsClosed = true;
	Head->Point->ToggleClickMeMode(false);
	Highlight();
}

void APolygon::SetLinkedProvider_Implementation(const TScriptInterface<UInteractionBoxProvider>& InProvider) {
	LinkedProvider = InProvider;
}

void APolygon::GetLinkedProvider_Implementation(TScriptInterface<UInteractionBoxProvider>& OutProvider) const {
	OutProvider = LinkedProvider;
}

void APolygon::GetInteractionBoxTitle_Implementation(FText& OutTitle) {
	if (LinkedProvider.GetObject() != nullptr) {
		IInteractionBoxProvider::Execute_GetInteractionBoxTitle(LinkedProvider.GetObject(), OutTitle);
	}
	else {
		OutTitle = FText::FromString("Polygon");
	}
}

void APolygon::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
	if (LinkedProvider.GetObject() != nullptr) {
		IInteractionBoxProvider::Execute_GetInteractionBoxKeyVals(LinkedProvider.GetObject(), OutKeyVals);
	}
}

void APolygon::ProvideOnPlaced_Implementation() {
	UObject* ProviderObj = LinkedProvider.GetObject();
	if (ProviderObj && ProviderObj->GetClass()->ImplementsInterface(UPlaceableProvider::StaticClass())) {
		IPlaceableProvider::Execute_ProvideOnPlaced(LinkedProvider.GetObject());
	}
	else {
		if (ProviderObj) {
			UE_LOG(LogTemp, Warning, TEXT("Linked provider does not implement PlaceableProvider"))
		}
	}
	if (IsClosed) {
		if (Inclusive) {
			Highlight();
		}
		else {
			OutlineOnly(true);
		}
	}
}

void APolygon::AddPoint(APlaceablePoint* InPoint) {
	IInteractionBoxProvider::Execute_SetLinkedProvider(InPoint, TScriptInterface<UInteractionBoxProvider>(this));
	TSharedPtr<FPolygonListNode> NewNode = MakeShared<FPolygonListNode>(InPoint);
	if (!Head.IsValid()) {
		Head = NewNode;
		Head->Next = NewNode;
		Head->Prev = NewNode;
	}
	else {
		TSharedPtr<FPolygonListNode> Tail = Head->Prev.Pin();
		Tail->Next = NewNode;
		NewNode->Prev = Tail;
		NewNode->Next = Head;
		Head->Prev = NewNode;
	}
	Length++;

	bool Closeable;
	CanBeClosed(Closeable);
	
	if (!IsClosed) {
		Head->Point->ToggleClickMeMode(Closeable);
		bool IsAlreadyBound = Head->Point->OnClickMeClick.IsAlreadyBound(this, &APolygon::CloseFromClickMe);

		if (Closeable && !IsAlreadyBound) {
			Head->Point->OnClickMeClick.AddDynamic(this, &APolygon::CloseFromClickMe);
		}
		else if (!Closeable && IsAlreadyBound) {
			Head->Point->OnClickMeClick.RemoveDynamic(this, &APolygon::CloseFromClickMe);
		}
	}
}

void APolygon::GetPointLocations(TArray<FVector>& OutLocations) {
	OutLocations.Empty();
	if (!Head.IsValid()) {
		return;
	}

	FPolygonListNode* Current = Head.Get();

	do {
		FVector LonLatHeight;
		Current->Point->GetLongitudeLatitudeHeight(LonLatHeight);
		OutLocations.Add(LonLatHeight);
		Current = Current->Next.Get();
	} while (Current != Head.Get());
}

void APolygon::GetActorLocations(TArray<FVector>& OutActorLocations) {
	OutActorLocations.Empty();
	if (!Head.IsValid()) {
		return;
	}

	FPolygonListNode* Current = Head.Get();

	do {
		FVector ActorLocat;
		ActorLocat = Current->Point->GetActorLocation();
		OutActorLocations.Add(ActorLocat);
		Current = Current->Next.Get();
	} while (Current != Head.Get());
	OutActorLocations.Add(Current->Point->GetActorLocation());
}

void APolygon::ForEachPoint(const TFunctionRef<void(APlaceablePoint*)>& Callback) const {
	if (!Head.IsValid()) {
		return;
	}

	FPolygonListNode* Current = Head.Get();

	do {
		Callback(Current->Point.Get());
		Current = Current->Next.Get();
	} while (Current != Head.Get());
}

void APolygon::OnInteractionBoxTitleChanged_Implementation(const FText &NewTitle) {
	ForEachPoint([NewTitle](APlaceablePoint* Point) {
		IInteractionBoxProvider::Execute_OnInteractionBoxTitleChanged(Point, NewTitle);
	});
}

void APolygon::ToggleCull_Implementation(bool IsCulled, bool FromFlyTo) {
	ForEachPoint([&IsCulled, &FromFlyTo](APlaceablePoint* Point) {
		IInteractionBoxProvider::Execute_ToggleCull(Point, IsCulled, FromFlyTo);
	});
}

FVector APolygon::GetCenterpointUE() const {
	FVector Sum = FVector::ZeroVector;
	ForEachPoint([&Sum](APlaceablePoint* Point) {
		Sum += Point->GetActorLocation();
	});
	return Sum / Length;
}

FVector APolygon::GetCenterpoint() const {
	FVector Sum = FVector::ZeroVector;
	ForEachPoint([&Sum](APlaceablePoint* Point) {
		FVector LonLatHeight;
		Point->GetLongitudeLatitudeHeight(LonLatHeight);
		Sum += LonLatHeight;
	});
	return Sum / Length;
}

bool APolygon::GetInclusive() const {
	return Inclusive;
}

void APolygon::SetInclusive(bool IsInclusive) {
	if (Inclusive != IsInclusive) {
		HighlightOnly(IsInclusive);
	}
	Inclusive = IsInclusive;
}
