/*
This file serves as a tutorial for writing unit tests using the spec framework.
As more and more features of the framework are used, examples should be added here for future reference
and to test that they work as expected.

IMPORTANT NOTE:
To get this file to integrate with Unreal Engine headers, I had to create it from within the engine
using "Add New C++ class...", select an empty class, make the class private, and click create.

Then, from within my filesystem (NOT FROM VISUAL STUDIO) I moved it to the `Private/Test` directory,
renamed the *.cpp file to *.spec.cpp, and deleted the header file.

After doing this, I was able to import Misc/AutomationTest.h and define Spec.
*/

// MUST: wrap every spec definition with this. We do not want to include tests in our release build
#if WITH_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include <EngineUtils.h>
// We can also include our own unreal header files here too


// These are classes necessary for examples
// In a real spec test, we would include these from our own header files
class ExampleFeature {
public:
	bool MakeBoolean(bool boolean) {
		return boolean;
	}
};

class ExampleFixture {
public:
	void IncrementBy(int value) {
		count += value;
	}

	int GetCount() const {
		return count;
	}

private:
	int count = 0;
};

class ExampleActor : public AActor {

private:

	bool exploded = false;

public:
	ExampleActor() {}

	void Damage(float DamageAmount)
	{
		if (DamageAmount > 0.0f)
		{
			exploded = true;
		}
	}

	bool HasExploded() const {
		return exploded;
	}
};

// Some classes like Actors require a reference to a world in order to be placed / tested / used.
// So, lets use what we know from ExampleFixture to make our own fixture that implements UWorld
class ExampleWorldFixture {
public:

	explicit ExampleWorldFixture(const FURL& URL = FURL()) {
		if (GEngine != nullptr) {
			static uint32 worldCounter = 0;
			const FString worldName = FString::Printf(TEXT("WorldFixture_%d"), worldCounter++);

			if (UWorld* world = UWorld::CreateWorld(EWorldType::Game, false, *worldName, GetTransientPackage())) {
				FWorldContext& worldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
				worldContext.SetCurrentWorld(world);

				world->InitializeActorsForPlay(URL);
				if (IsValid(world->GetWorldSettings())) {
					// Need to do this manually since world doesn't have a game mode
					world->GetWorldSettings()->NotifyBeginPlay();
					world->GetWorldSettings()->NotifyMatchStarted();
				}
				world->BeginPlay();

				weakWorld = MakeWeakObjectPtr(world);
			}
		}
	}

	UWorld* GetWorld() const {
		return weakWorld.Get();
	}

	~ExampleWorldFixture() {
		UWorld* world = weakWorld.Get();
		if (world != nullptr && GEngine != nullptr) {
			world->BeginTearingDown();

			// Make sure to clean up all actors immediately
			// Destroy world doesn't do this and instead waits for GC to clear everything up
			for (auto it : TActorRange<AActor>(world)) {
				it->Destroy();
			}

			GEngine->DestroyWorldContext(world);
			world->DestroyWorld(false);
		}
	}

private:

	TWeakObjectPtr<UWorld> weakWorld;
};


// This creates our spec class which facilitates running all of our tests
BEGIN_DEFINE_SPEC(FExampleSpec, "Example", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

// Variables and functions defined here will end up being member of 
// the FExampleSpec class and will be accessible in the tests
TUniquePtr<ExampleFixture> exampleFixture;

TUniquePtr<ExampleWorldFixture> worldFixture;
TWeakObjectPtr<ExampleActor> exampleActorFixture;

END_DEFINE_SPEC(FExampleSpec);


// Test definition for current spec
void FExampleSpec::Define() {

	// Test cases are defined here
	Describe("ExampleFeature", [this]() {
		It("Should return true when input is true", [this]() {
			ExampleFeature feature;
			TestTrue("MakeBoolean", feature.MakeBoolean(true));
		});

		It("Should return false when input is false", [this]() {
			ExampleFeature feature;
			TestFalse("MakeBoolean", feature.MakeBoolean(false));
		});
	});

	Describe("ExampleFixture", [this]() {
		BeforeEach([this]() {

			// Set up a fixture before each "It" test in this "Describe" block
			exampleFixture = MakeUnique<ExampleFixture>();

			// If the setup of this fixture fails, no "It" tests in this "Describe" block will pass
			// This is pretty important and should always be added
			TestTrue("exampleFixture valid", exampleFixture.IsValid());
		});

		AfterEach([this]() {
			// Destroy the fixture
			exampleFixture.Reset();
		});

		It("Should start from zero", [this]() {
			TestEqual("GetCount", exampleFixture->GetCount(), 0);

			// Even though we increment by 1 here, count will be 0 in the next "It" test because the fixture resets
			exampleFixture->IncrementBy(1);
		});

		It("Should increment by provided number", [this]() {
			const int count = 3;
			exampleFixture->IncrementBy(count);
			TestEqual("GetCount", exampleFixture->GetCount(), 3);
		});
	});

	Describe("ExampleActor", [this]() {
		BeforeEach([this]() {
			// Create the world
			worldFixture = MakeUnique<ExampleWorldFixture>();

			if (TestNotNull("World", worldFixture->GetWorld())) {
				// Spawn the actor
				exampleActorFixture = worldFixture->GetWorld()->SpawnActor<ExampleActor>();
				TestNotNull("ExampleActor", exampleActorFixture.Get());
			}
		});

		AfterEach([this]() {
			// Tear down the world and the actor
			worldFixture.Reset();
		});

		It("Should not explode when dealt zero damage", [this]() {
			exampleActorFixture->Damage(0.0);
			TestFalse("HasExploded", exampleActorFixture->HasExploded());
		});

		It("Should explode when dealt damage", [this]() {
			exampleActorFixture->Damage(1.0);
			TestTrue("HasExploded", exampleActorFixture->HasExploded());
		});
	});
}

#endif
