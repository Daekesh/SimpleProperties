# SimpleProperties
A c++ based advanced property plugin for Unreal Engine

# Blurb
I thought to myself the other day, after looking at some code at work, "Wouldn't it be nice to have some way to remove 90% of this boilerplate?" So, over the weekend, I came up with version one of this plugin.

# Aims
1. To reduce boilerplate for properties and make them more like c# properties
2. Blueprint integration
3. Automatic "on changed" function calls
4. Advanced, unreal-specific integrations
5. Eventually, some sort of optimisation?

# What it does (currently)
- Assign and use values as if you weren't using the wrapper
- Automatic type conversion to expose inner value (operator*, operator->, operator type&()).
- Automatic type conversion is customisation per-type for both pointer and reference types.
- A const version so that you can expose the properties, but require a secret key type to set them.
- Equality checks when a value is set. If the value is different call an update function (this may be computationally expensive, I don't really care. Don't use it for everything?)
- Ability to set on change callback method, callback object and parameters when creating the object. The method can be changed later.
- Ability to override the equality operator for the above.
- When used with TWeakPtr, TSharedPtr and TSharedRef, automatically dereferences the smart pointer to access the inner value.
- Built in managed, unmanaged and scoped transaction support.
- Automatically calls Modify on contained UObject* TObjectPtr<> values
- Unreal archive support
- Instanced-struct style inner property lookup for UObjects.
- Ability to customse GC (AddStructReferencedObjects)
- Automatic conversion of initializer-list style callback for any type of callback function/lambda with any number of arguments.

# Example code
- Base class example
```cpp
TSimplePropertyBase<FString> BaseStringProp = "Hello";
TSimplePropertyBase<FString> BaseStringProp2 = BaseStringProp;
TSimplePropertyBase<FString> BaseStringProp3 = TSimplePropertyBase<FString>{"Moo"};
```

- Private key example:
```cpp
TSimpleConstProperty<float, FPrivateToken> FloatProp2 = FloatProp;
FloatProp2.Set({}, 50);
FloatProp2.Set({}, FloatProp);
```

- Conversion
```cpp
FString Foo = "Foo";
TSimplePropertyBase<FString> BaseStringProp4 = Foo;
Foo = BaseStringProp;
IntProp = (int32)*BoolProp;
```

- Callback initialisation
```cpp
TSimpleProperty<FVector> VectorProperty6 = {FVector::ZeroVector, {this, &FSimplePropertiesModule::OnChangeWithParams, false, 67}};
VectorProperty6->X = 5;
VectorProperty6.OnChange();
```

- TSharedPtr
```cpp
TSimpleProperty<TSharedPtr<FVector>> SharedVector = MakeShared<FVector>();
SharedVector->Y = 5;
SharedVector.OnChange(); // Only calls change when the entire value is changed, not the sub properties. Limitations of c++?
```

- Transaction
```cpp
const bool bStartedTransaction = ColorProperty.StartManagedTransaction(INVTEXT("Test Managed Transaction"));
const bool bHasManagedTransaction = VectorProperty6.HasManagedTransaction();
ColorProperty = FColor::Blue;
ESimplePropertyTransactionEndResult Result1 = ColorProperty.EndManagedTransaction(true);
```

```cpp
int32 Transaction = VectorProperty6.StartUnmanagedTransaction(INVTEXT("Test Unmanaged Transaction"));
const bool bHasBeenModified = VectorProperty6.IsModified();
VectorProperty6 = FVector{5, 6, 7};
const bool bHasBeenModified2 = VectorProperty6.IsModified();
const ESimplePropertyTransactionEndResult EndedTransactionResult = VectorProperty6.EndUnmanagedTransaction();
```

```cpp
TSharedPtr<FScopedTransaction> ScopedTransaction = VectorProperty6.StartUnmanagedScopedTransaction(INVTEXT("Test Unmanaged Scoped Transaction"));
const bool bHasBeenModifie3 = VectorProperty6.IsModified();
VectorProperty6 = FVector{1, 2, 3};
const bool bHasBeenModified4 = VectorProperty6.IsModified(); // Should be false because scoped transactions offer no local change tracking
```

# TODO
- BP integration
