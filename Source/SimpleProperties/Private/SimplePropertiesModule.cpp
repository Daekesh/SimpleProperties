// Copyright Matt Chapman. All Rights Reserved.

#include "Modules/ModuleInterface.h"
#include "GameFramework/Actor.h"
#include "Modules/ModuleManager.h"
#include "SimpleProperty.h"

#if WITH_EDITOR
#include "ScopedTransaction.h"
#endif

namespace UE::SimpleProperties::Private
{
	void SimpleGlobalFunction()
	{
	}
}

class FSimplePropertiesModule : public IModuleInterface
{
private:
	// Used for public-read, private-write properties
	struct FPrivateToken {};

public:
	virtual void StartupModule() override
	{
		PerformTests();
	}

	virtual void ShutdownModule() override
	{
	}

protected:
	static void OnStaticChange()
	{
	}

	void OnChange()
	{
	}

	void OnChangeWithParams(bool bInTest, int32 InNumber)
	{
	}

	void PerformTests()
	{
		TSimpleProperty<FString> StringProp = TEXT("Moo");

		{
			TSimpleProperty<FString> ScopedProp = TEXT("Foo");
			StringProp = ScopedProp;
		}

		UE_LOG(LogTemp, Log, TEXT("%s"), **StringProp);

		// Template check
		if (false)
		{
			TObjectPtr<AActor> Test = nullptr;
			FReferenceCollector* Collector = nullptr;
			TSimplePropertyTypeTraits<TObjectPtr<AActor>>::FReferenceCollectorType::AddReferences(Test, *Collector);
			TSimplePropertyTypeTraits<AActor*>::FReferenceCollectorType::AddReferences(Test.Get(), *Collector);
		}

		using namespace UE::SimpleProperties::Private;

		FSimplePropertyOnChange Test = {FSimplePropertyOnChange::FDelegate::CreateLambda([]() {})};
		FSimplePropertyOnChange Test2 = {[](){}};
		FSimplePropertyOnChange Test3 = {&SimpleGlobalFunction};
		FSimplePropertyOnChange Test4 = {&FSimplePropertiesModule::OnStaticChange};
		FSimplePropertyOnChange Test5 = {this, &FSimplePropertiesModule::OnChange};
		FSimplePropertyOnChange Test6 = {this, &FSimplePropertiesModule::OnChangeWithParams, false, 67};

		TSimpleProperty<bool> BoolProp = false;
		TSimpleProperty<int32> IntProp{10};

		IntProp = (int32)*BoolProp;

		TSimplePropertyBase<FString> BaseStringProp = "Hello";
		TSimplePropertyBase<FString> BaseStringProp2 = BaseStringProp;
		TSimplePropertyBase<FString> BaseStringProp3 = TSimplePropertyBase<FString>{"Moo"};

		FString Foo = "Foo";
		TSimplePropertyBase<FString> BaseStringProp4 = Foo;
		Foo = BaseStringProp;

		IntProp = BaseStringProp4->Len();

		TSimpleConstProperty<float> FloatProp = 5.f;

		// Allows in-class setting 
		TSimpleConstProperty<float, FPrivateToken> FloatProp2 = FloatProp;
		FloatProp2.Set({}, 50);
		FloatProp2.Set({}, FloatProp);

		TSimpleConstProperty<bool> ConstBoolProp = false;
		TSimpleConstProperty<bool> ConstBoolProp2 = ConstBoolProp;
		TSimpleConstProperty<bool> ConstBoolProp3 = TSimpleConstProperty<bool>{true};

		TSimpleConstProperty<FVector> ConstVectorProperty = {FVector(1, 2, 3)};
		TSimpleConstProperty<FVector> ConstVectorProperty2 = {FVector::ZeroVector};
		TSimpleConstProperty<FVector> ConstVectorProperty3 = {FVector::ZeroVector, &SimpleGlobalFunction};
		TSimpleConstProperty<FVector> ConstVectorProperty4 = {FVector::ZeroVector, &FSimplePropertiesModule::OnStaticChange};
		TSimpleConstProperty<FVector> ConstVectorProperty5 = {FVector::ZeroVector, {this, &FSimplePropertiesModule::OnChange}};
		TSimpleConstProperty<FVector> ConstVectorProperty6 = {FVector::ZeroVector, {this, &FSimplePropertiesModule::OnChangeWithParams, false, 67}};

		TSimpleProperty<FVector> VectorProperty = FVector(1, 2, 3);
		TSimpleProperty<FVector> VectorProperty1 = {FVector(1, 2, 3)};
		TSimpleProperty<FVector> VectorProperty2 = {FVector::ZeroVector};
		TSimpleProperty<FVector> VectorProperty3 = {FVector::ZeroVector, &SimpleGlobalFunction};
		TSimpleProperty<FVector> VectorProperty4 = {FVector::ZeroVector, &FSimplePropertiesModule::OnStaticChange};
		TSimpleProperty<FVector> VectorProperty5 = {FVector::ZeroVector, {this, &FSimplePropertiesModule::OnChange}};
		TSimpleProperty<FVector> VectorProperty6 = {FVector::ZeroVector, {this, &FSimplePropertiesModule::OnChangeWithParams, false, 67}};
		VectorProperty6->X = 5;
		VectorProperty6.OnChange(); // Accessing members does not trigger setting the whole value, so trigger it manually

		VectorProperty = ConstVectorProperty;
		VectorProperty2 = VectorProperty3;
		//ConstVectorProperty4 = VectorProperty4; // Can't assign to const property (see above)

		TSimpleProperty<FColor> ColorProperty = TSimplePropertyBase<FColor>(FColor::Red);
		ColorProperty = FColor::Green;

		TSimpleProperty<TSharedPtr<FVector>> SharedVector = MakeShared<FVector>();
		SharedVector->Y = 5;
		SharedVector.OnChange();

		const bool bValid = (*SharedVector).IsValid();

#if WITH_EDITOR
		const bool bStartedTransaction = ColorProperty.StartManagedTransaction(INVTEXT("Test Managed Transaction"));
		const bool bHasManagedTransaction = VectorProperty6.HasManagedTransaction();
		ColorProperty = FColor::Blue;
		ESimplePropertyTransactionEndResult Result1 = ColorProperty.EndManagedTransaction(true);

		//VectorProperty6.StartUnmanagedTransaction(INVTEXT("Test Unmanaged Transaction")); // No discard error

		{
			int32 Transaction = VectorProperty6.StartUnmanagedTransaction(INVTEXT("Test Unmanaged Transaction"));
			const bool bHasBeenModified = VectorProperty6.IsModified();
			VectorProperty6 = FVector{5, 6, 7};
			const bool bHasBeenModified2 = VectorProperty6.IsModified();
			const ESimplePropertyTransactionEndResult EndedTransactionResult = VectorProperty6.EndUnmanagedTransaction();

			TSharedPtr<FScopedTransaction> ScopedTransaction = VectorProperty6.StartUnmanagedScopedTransaction(INVTEXT("Test Unmanaged Scoped Transaction"));
			const bool bHasBeenModifie3 = VectorProperty6.IsModified();
			VectorProperty6 = FVector{1, 2, 3};
			const bool bHasBeenModified4 = VectorProperty6.IsModified(); // Should be false because scoped transactions offer no local change tracking
			// Cannot end transaction
		}

		const bool bHasBeenModified3 = VectorProperty6.IsModified();
#endif
	}
};


IMPLEMENT_MODULE(FSimplePropertiesModule, SimpleProperties)
