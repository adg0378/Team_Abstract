// Copyright (c) 2025 Synetos Aerospace


#include "Objects/Geo/SPPolygon.h"
#include "Objects/Geo/SPPlaceablePoint.h"
#include "Core/State/SPGameState.h"
#include "Core/State/SPPlacementManager.h"

#undef GetObject

void ASPPolygon::DestroySelf_Implementation() {
	if (LinkedProvider.GetObject() != nullptr) {
		ISPInteractionInterface::Execute_DestroySelf(LinkedProvider.GetObject());
	}
	else {
		DestroyPolygon();
	}
}

void ASPPolygon::DestroyPolygon() {
	if (IsClosed) {
		UnHighlight();
	}
	ClearAllPoints();
	Destroy();
}

void ASPPolygon::ClearAllPoints() {
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

void ASPPolygon::CanBeClosed(bool& OutCanBeClosed) const {
	OutCanBeClosed = Length >= 3;
}

ASPPlaceablePoint* ASPPolygon::GetHead() const {
	return Head->Point;
}

void ASPPolygon::Highlight_Implementation() {

}

void ASPPolygon::UnHighlight_Implementation() {

}

void ASPPolygon::CloseFromClickMe() {
	Head->Point->OnClickMeClick.RemoveDynamic(this, &ASPPolygon::CloseFromClickMe);
	Close();
	ASPGameState::GetSPGameState(this)->PlacementManager->SpawnActor();
}

void ASPPolygon::Close() {
	IsClosed = true;
	Head->Point->ToggleClickMeMode(false);
	Highlight();
}

void ASPPolygon::SetLinkedProvider_Implementation(const TScriptInterface<USPInteractionInterface>& InProvider) {
	LinkedProvider = InProvider;
}

TScriptInterface<USPInteractionInterface> ASPPolygon::GetLinkedProvider_Implementation() const {
	return LinkedProvider;
}

void ASPPolygon::GetInteractionBoxTitle_Implementation(FText& OutTitle) {
	if (LinkedProvider.GetObject() != nullptr) {
		ISPInteractionInterface::Execute_GetInteractionBoxTitle(LinkedProvider.GetObject(), OutTitle);
	}
	else {
		OutTitle = FText::FromString("Polygon");
	}
}

void ASPPolygon::GetInteractionBoxKeyVals_Implementation(TMap<FString, FInteractionBoxValue>& OutKeyVals) {
	if (LinkedProvider.GetObject() != nullptr) {
		ISPInteractionInterface::Execute_GetInteractionBoxKeyVals(LinkedProvider.GetObject(), OutKeyVals);
	}
}

void ASPPolygon::ProvideOnPlaced_Implementation() {
	UObject* ProviderObj = LinkedProvider.GetObject();
	if (ProviderObj && ProviderObj->GetClass()->ImplementsInterface(USPPlaceableInterface::StaticClass())) {
		ISPPlaceableInterface::Execute_ProvideOnPlaced(LinkedProvider.GetObject());
	}
	else {
		if (ProviderObj) {
			UE_LOG(LogTemp, Warning, TEXT("Linked provider does not implement PlaceableProvider"))
		}
	}
	if (IsClosed) {
		Highlight();
	}
}

void ASPPolygon::AddPoint(ASPPlaceablePoint* InPoint) {
	ISPInteractionInterface::Execute_SetLinkedProvider(InPoint, TScriptInterface<USPInteractionInterface>(this));
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
		bool IsAlreadyBound = Head->Point->OnClickMeClick.IsAlreadyBound(this, &ASPPolygon::CloseFromClickMe);

		if (Closeable && !IsAlreadyBound) {
			Head->Point->OnClickMeClick.AddDynamic(this, &ASPPolygon::CloseFromClickMe);
		}
		else if (!Closeable && IsAlreadyBound) {
			Head->Point->OnClickMeClick.RemoveDynamic(this, &ASPPolygon::CloseFromClickMe);
		}
	}
}

void ASPPolygon::GetPointLocations(TArray<FVector>& OutLocations) {
	OutLocations.Empty();
	if (!Head.IsValid()) {
		return;
	}

	FPolygonListNode* Current = Head.Get();

	do {
		OutLocations.Add(Current->Point->GetLongitudeLatitudeHeight());
		Current = Current->Next.Get();
	} while (Current != Head.Get());
}

void ASPPolygon::GetActorLocations(TArray<FVector>& OutActorLocations) {
	OutActorLocations.Empty();
	if (!Head.IsValid()) {
		return;
	}

	FPolygonListNode* Current = Head.Get();

	do {
		OutActorLocations.Add(Current->Point->GetActorLocation());
		Current = Current->Next.Get();
	} while (Current != Head.Get());
	OutActorLocations.Add(Current->Point->GetActorLocation());
}

void ASPPolygon::ForEachPoint(const TFunctionRef<void(ASPPlaceablePoint*)>& Callback) const {
	if (!Head.IsValid()) {
		return;
	}

	FPolygonListNode* Current = Head.Get();

	do {
		Callback(Current->Point.Get());
		Current = Current->Next.Get();
	} while (Current != Head.Get());
}

void ASPPolygon::OnInteractionBoxTitleChanged_Implementation(const FText& NewTitle) {
	ForEachPoint([NewTitle](ASPPlaceablePoint* Point) {
		ISPInteractionInterface::Execute_OnInteractionBoxTitleChanged(Point, NewTitle);
		});
}

void ASPPolygon::ToggleCull_Implementation(bool IsCulled) {
	ForEachPoint([&IsCulled](ASPPlaceablePoint* Point) {
		ISPInteractionInterface::Execute_ToggleCull(Point, IsCulled);
		});
}

FVector ASPPolygon::GetCenterpointUE() const {
	FVector Sum = FVector::ZeroVector;
	ForEachPoint([&Sum](ASPPlaceablePoint* Point) {
		Sum += Point->GetActorLocation();
		});
	return Sum / Length;
}

FVector ASPPolygon::GetCenterpoint() const {
	FVector Sum = FVector::ZeroVector;
	ForEachPoint([&Sum](ASPPlaceablePoint* Point) {
		Sum += Point->GetLongitudeLatitudeHeight();
		});
	return Sum / Length;
}
