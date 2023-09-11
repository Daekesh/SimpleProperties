// Copyright Matt Chapman. All Rights Reserved.

#pragma once

#include "SimplePropertyConcepts.h"
#include "SimplePropertyEvents.h"
#include "SimplePropertyTypeTraits.h"
#include "Templates/Models.h"
#include "Templates/UnrealTypeTraits.h"
#include "UObject/Class.h"

#if WITH_EDITOR
#include "Internationalization/Text.h"
#include "SimplePropertyTransactionManager.h"
#endif

namespace UE::SimpleProperties
{
	using NoType = decltype(nullptr);
}

#if WITH_EDITOR
enum class ESimplePropertyTransactionEndResult : uint8
{
	Cancelled,
	Ended,
	Invalid,
	NotManaged,
	NotUnmanaged,
	UnknownError,
	FirstError = Invalid
};
#endif

// Base class defining the container and the data.
template<typename InValueType>
struct TSimplePropertyBase
{
	using FValueTypes = TSimplePropertyTypeTraits<InValueType>::template FValueTypes;
	using FValueType = FValueTypes::template FValueType;
	using FReferenceType = FValueTypes::template FReferenceType;
	using FPointerType = FValueTypes::template FPointerType;

	TSimplePropertyBase()
	{
		static_assert(TOr<
				TIsPointer<FValueType>,
				TNot<TIsDerivedFrom<typename TRemovePointer<FValueType>::Type, UObject>>
			>::Value
		);
	}

	TSimplePropertyBase(const TSimplePropertyBase& InOther)
	{
		Value = InOther.Value;
	}

	TSimplePropertyBase(TSimplePropertyBase&& InOther)
	{
		Value = MoveTemp(InOther.Value);
	}

	template<typename InAssignType>
	TSimplePropertyBase(const InAssignType& InDefaultValue)
		: Value(InDefaultValue)
	{
	}

	template<typename InAssignType>
	TSimplePropertyBase(InAssignType&& InDefaultValue)
		: Value(MoveTemp(InDefaultValue))
	{
	}

	operator const FReferenceType&()
	{
		return FValueTypes::GetReferenceValue(Value);
	}

	const FReferenceType& operator*()
	{
		return FValueTypes::GetReferenceValue(Value);
	}

	const FPointerType* operator->()
	{
		return FValueTypes::GetPointerValue(Value);
	}

	friend FArchive& operator<<(FArchive& InArchive, const TSimplePropertyBase& InProperty)
	{
		InArchive << InProperty.Value;
		return InArchive;
	}

	void AddStructReferencedObjects(FReferenceCollector& InCollector)
	{
		TSimplePropertyTypeTraits<FValueType>::FReferenceCollectorType::AddReferences(Value, InCollector);
	}

	bool FindInnerPropertyInstance(FName InPropertyName, const FProperty*& OutProp, const void*& OutData) const
	{
		return TSimplePropertyTypeTraits<FValueType>::FFindInnerPropertyType::FindInnerPropertyInstance(
			Value, InPropertyName, OutProp, OutData);
	}

protected:
	FValueType Value;
};

// Const property that requires a private key type to set (define in owning class)
template<typename InValueType, typename InPrivateType = UE::SimpleProperties::NoType
	UE_REQUIRES(TOr<TOr<
			TModels<CEqualityComparable, InValueType>,
			TModels<CEqualityEquals, InValueType>>,
			TModels<CEqualityEqualTo, InValueType>>
		::Value)>
	struct TSimpleConstProperty : public TSimplePropertyBase<InValueType>
{
	using Base = TSimplePropertyBase<InValueType>;
	using FValueType = Base::template FValueType;
	using FPrivateType = InPrivateType;
	using FComparatorType = TSimplePropertyTypeTraits<FValueType>::template FComparatorType;

#if WITH_EDITOR
	using FTransactionObjectType = TSimplePropertyTypeTraits<FValueType>::template FTransactionObjectType;
#endif

	TSimpleConstProperty()
		: Base()
		, OnChangeDelegate(FSimplePropertyOnChange::FDelegate())
	{
	}

	TSimpleConstProperty(const TSimpleConstProperty& InOther)
		: Base(InOther.Value)
		, OnChangeDelegate(InOther.OnChangeDelegate)
#if WITH_EDITOR
		, bModifiedInTransaction(InOther.bModifiedInTransaction)
#endif
	{
	}

	TSimpleConstProperty(TSimpleConstProperty&& InOther)
		: Base(Forward<FValueType>(InOther.Value))
		, OnChangeDelegate(MoveTemp(InOther.OnChangeDelegate))
#if WITH_EDITOR
		, TransactionId(InOther.TransactionId)
		, bModifiedInTransaction(InOther.bModifiedInTransaction)
#endif
	{
	}

	TSimpleConstProperty(FSimplePropertyOnChange&& InUpdateFunc)
		: Base()
		, OnChangeDelegate(MoveTemp(InUpdateFunc.Callback))
	{
	}

	template<typename InAssignType
		UE_REQUIRES(std::negation_v<std::is_same<InAssignType, FSimplePropertyOnChange>>)>
	TSimpleConstProperty(const InAssignType& InDefaultValue)
		: Base(InDefaultValue)
		, OnChangeDelegate(FSimplePropertyOnChange::FDelegate())
	{
	}

	template<typename InAssignType
		UE_REQUIRES(std::negation_v<std::is_same<InAssignType, FSimplePropertyOnChange>>)>
	TSimpleConstProperty(InAssignType&& InDefaultValue)
		: Base(Forward<InAssignType>(InDefaultValue))
		, OnChangeDelegate(FSimplePropertyOnChange::FDelegate())
	{
	}

	template<typename InAssignType
		UE_REQUIRES(std::negation_v<std::is_same<InAssignType, FSimplePropertyOnChange>>)>
	TSimpleConstProperty(const InAssignType& InDefaultValue, FSimplePropertyOnChange&& InUpdateFunc)
		: Base(InDefaultValue)
		, OnChangeDelegate(MoveTemp(InUpdateFunc.Callback))
	{
	}

	template<typename InAssignType
		UE_REQUIRES(std::negation_v<std::is_same<InAssignType, FSimplePropertyOnChange>>)>
	TSimpleConstProperty(InAssignType&& InDefaultValue, FSimplePropertyOnChange&& InUpdateFunc)
		: Base(Forward<InAssignType>(InDefaultValue))
		, OnChangeDelegate(MoveTemp(InUpdateFunc.Callback))
	{
	}

	// Allow private set access
	template<typename InAssignType>
	bool Set(FPrivateType Private, const InAssignType& InValue)
	{
		static_assert(!std::is_same_v<FPrivateType, UE::SimpleProperties::NoType>);
		return SetInternal(InValue);
	}

	template<typename InAssignType>
	bool Set(FPrivateType Private, InAssignType&& InValue)
	{
		static_assert(!std::is_same_v<FPrivateType, UE::SimpleProperties::NoType>);
		return SetInternal(MoveTemp(InValue));
	}

	template<typename InAssignType>
	bool Set(FPrivateType Private, const TSimpleConstProperty<InAssignType>& InProperty)
	{
		static_assert(!std::is_same_v<FPrivateType, UE::SimpleProperties::NoType>);
		return SetInternal(InProperty.Value);
	}

	template<typename InAssignType>
	bool Set(FPrivateType Private, TSimpleConstProperty<InAssignType>&& InProperty)
	{
		static_assert(!std::is_same_v<FPrivateType, UE::SimpleProperties::NoType>);
		return SetInternal(MoveTemp(InProperty.Value));
	}

	void OnChange()
	{
		OnChangeDelegate.ExecuteIfBound();

#if WITH_EDITOR
		if (FSimplePropertyTransactionManager::IsValidTransactionId(TransactionId))
		{
			bModifiedInTransaction = true;
		}
#endif
	}

	void SetOnChange(FSimplePropertyOnChange::FDelegate InCallback)
	{
		OnChangeDelegate = InCallback;
	}

#if WITH_EDITOR
	bool StartManagedTransaction(const FText& InTransactionDescription)
	{
		if (FSimplePropertyTransactionManager::IsValidTransactionId(TransactionId))
		{
			return false;
		}

		UObject* Object = FTransactionObjectType::GetTransactionObject(Base::Value);

		const int32 NewTransactionId = FSimplePropertyTransactionManager::Get().StartManagedTransaction(
			InTransactionDescription, Object);

		if (!FSimplePropertyTransactionManager::IsValidTransactionId(NewTransactionId))
		{
			return false;
		}

		if (Object)
		{
			Object->Modify();
		}

		TransactionId = NewTransactionId;
		bModifiedInTransaction = false;

		return true;
	}

	UE_NODISCARD int32 StartUnmanagedTransaction(const FText& InTransactionDescription)
	{
		if (FSimplePropertyTransactionManager::IsValidTransactionId(TransactionId))
		{
			return FSimplePropertyTransactionManager::GetInvalidTransactionId();
		}

		UObject* Object = FTransactionObjectType::GetTransactionObject(Base::Value);

		const int32 NewTransactionId = FSimplePropertyTransactionManager::Get().StartUnmanagedTransaction(
			InTransactionDescription, Object);

		if (!FSimplePropertyTransactionManager::IsValidTransactionId(NewTransactionId))
		{
			return FSimplePropertyTransactionManager::GetInvalidTransactionId();
		}

		if (Object)
		{
			Object->Modify();
		}

		TransactionId = NewTransactionId;
		bModifiedInTransaction = false;

		return true;
	}

	// Has no local tracking support!
	UE_NODISCARD TSharedPtr<FScopedTransaction> StartUnmanagedScopedTransaction(const FText& InTransactionDescription)
	{
		if (FSimplePropertyTransactionManager::IsValidTransactionId(TransactionId))
		{
			return nullptr;
		}

		UObject* Object = FTransactionObjectType::GetTransactionObject(Base::Value);

		TSharedPtr<FScopedTransaction> NewTransaction = FSimplePropertyTransactionManager::Get().StartUnmanagedScopedTransaction(
			InTransactionDescription, Object);

		if (!NewTransaction || !NewTransaction->IsOutstanding())
		{
			return nullptr;
		}

		if (Object)
		{
			Object->Modify();
		}

		bModifiedInTransaction = false;

		return NewTransaction;
	}

	ESimplePropertyTransactionEndResult CancelManagedTransaction()
	{
		bModifiedInTransaction = false;

		if (!FSimplePropertyTransactionManager::IsValidTransactionId(TransactionId))
		{
			TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
			return ESimplePropertyTransactionEndResult::Invalid;
		}

		if (!ensure(FSimplePropertyTransactionManager::Get().IsManagedTransaction(TransactionId)))
		{
			TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
			return ESimplePropertyTransactionEndResult::NotManaged;
		}

		if (!FSimplePropertyTransactionManager::Get().CancelManagedTransaction(TransactionId))
		{
			TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
			return ESimplePropertyTransactionEndResult::UnknownError;
		}

		TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
		return ESimplePropertyTransactionEndResult::Cancelled;
	}

	ESimplePropertyTransactionEndResult EndManagedTransaction(bool bInCancelIfUnmodified = false)
	{
		if (!FSimplePropertyTransactionManager::IsValidTransactionId(TransactionId))
		{
			bModifiedInTransaction = false;
			TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
			return ESimplePropertyTransactionEndResult::Invalid;
		}

		if (!ensure(FSimplePropertyTransactionManager::Get().IsManagedTransaction(TransactionId)))
		{
			bModifiedInTransaction = false;
			TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
			return ESimplePropertyTransactionEndResult::NotManaged;
		}

		if (bInCancelIfUnmodified && !bModifiedInTransaction)
		{
			if (!FSimplePropertyTransactionManager::Get().CancelManagedTransaction(TransactionId))
			{
				bModifiedInTransaction = false;
				TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
				return ESimplePropertyTransactionEndResult::UnknownError;
			}
		}
		else
		{
			if (!FSimplePropertyTransactionManager::Get().EndManagedTransaction(TransactionId))
			{
				bModifiedInTransaction = false;
				TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
				return ESimplePropertyTransactionEndResult::UnknownError;
			}
		}

		bModifiedInTransaction = false;
		TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
		return ESimplePropertyTransactionEndResult::Ended;
	}

	ESimplePropertyTransactionEndResult CancelUnmanagedTransaction()
	{
		bModifiedInTransaction = false;

		if (!FSimplePropertyTransactionManager::IsValidTransactionId(TransactionId))
		{
			TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
			return ESimplePropertyTransactionEndResult::Invalid;
		}

		if (!ensure(FSimplePropertyTransactionManager::Get().IsUnmanagedTransaction(TransactionId)))
		{
			TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
			return ESimplePropertyTransactionEndResult::NotUnmanaged;
		}

		if (!FSimplePropertyTransactionManager::Get().CancelUnmanagedTransaction(TransactionId))
		{
			TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
			return ESimplePropertyTransactionEndResult::UnknownError;
		}

		TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
		return ESimplePropertyTransactionEndResult::Cancelled;
	}

	ESimplePropertyTransactionEndResult EndUnmanagedTransaction(bool bInCancelIfUnmodified = false)
	{
		if (!FSimplePropertyTransactionManager::IsValidTransactionId(TransactionId))
		{
			bModifiedInTransaction = false;
			TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
			return ESimplePropertyTransactionEndResult::Invalid;
		}

		if (!ensure(FSimplePropertyTransactionManager::Get().IsUnmanagedTransaction(TransactionId)))
		{
			bModifiedInTransaction = false;
			TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
			return ESimplePropertyTransactionEndResult::NotUnmanaged;
		}

		if (bInCancelIfUnmodified && !bModifiedInTransaction)
		{
			if (!FSimplePropertyTransactionManager::Get().CancelUnmanagedTransaction(TransactionId))
			{
				bModifiedInTransaction = false;
				TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
				return ESimplePropertyTransactionEndResult::UnknownError;
			}
		}
		else
		{
			if (FSimplePropertyTransactionManager::Get().EndUnmanagedTransaction(TransactionId))
			{
				bModifiedInTransaction = false;
				TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
				return ESimplePropertyTransactionEndResult::Ended;
			}
		}

		bModifiedInTransaction = false;
		TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
		return ESimplePropertyTransactionEndResult::UnknownError;
	}

	bool HasManagedTransaction() const
	{
		return FSimplePropertyTransactionManager::Get().IsManagedTransaction(TransactionId);
	}

	bool HasUnmangaedTransaction() const
	{
		return FSimplePropertyTransactionManager::IsUnmanagedTransaction(TransactionId);
	}

	/** Returns true if we have a an active transaction and we've been modified. */
	bool IsModified() const
	{
		return bModifiedInTransaction;
	}
#endif

protected:
	FSimplePropertyOnChange::FDelegate OnChangeDelegate;

#if WITH_EDITOR
	int32 TransactionId = FSimplePropertyTransactionManager::GetInvalidTransactionId();
	bool bModifiedInTransaction = false;
#endif

	template<typename InCompareType>
	bool IsEqual(const InCompareType& InOther)
	{
		return FComparatorType::IsEqual<FValueType, InCompareType>(Base::Value, InOther);
	}

	template<typename InAssignType>
	bool SetInternal(const InAssignType& InValue)
	{
		if (IsEqual(InValue))
		{
			return false;
		}

		Base::Value = InValue;

		OnChange();

		return true;
	}

	template<typename InAssignType>
	bool SetInternal(InAssignType&& InValue)
	{
		if (IsEqual(InValue))
		{
			return false;
		}

		Base::Value = MoveTemp(InValue);

		OnChange();

		return true;
	}
};

// Can be get and set by anything
template<typename InValueType, typename InPrivateType = UE::SimpleProperties::NoType
	UE_REQUIRES(TOr<TOr<
			TModels<CEqualityComparable, InValueType>, 
			TModels<CEqualityEquals, InValueType>>, 
			TModels<CEqualityEqualTo, InValueType>>
		::Value)>
struct TSimpleProperty : public TSimpleConstProperty<InValueType, InPrivateType>
{
	using Super = TSimpleConstProperty<InValueType, InPrivateType>;
	using Base = Super::template Base;
	using FValueType = Base::template FValueType;
	using FValueTypes = Base::template FValueTypes;
	using FReferenceType = Base::template FReferenceType;
	using FPointerType = Base::template FPointerType;

	TSimpleProperty()
		: Super()
	{
	}

	TSimpleProperty(const TSimpleProperty& InOther)
		: Super(InOther.Value, InOther.OnChangeDelegate)
	{
	}

	TSimpleProperty(TSimpleProperty&& InOther)
		: Super(Forward<FValueType>(InOther.Value), Forward<Super::FOnCallback>(InOther.OnChangeDelegate))
	{
	}

	TSimpleProperty(FSimplePropertyOnChange&& InUpdateFunc)
		: Super(Forward<Super::FOnCallback>(InUpdateFunc.Callback))
	{
	}

	template<typename InAssignType
		UE_REQUIRES(std::negation_v<std::is_same<InAssignType, FSimplePropertyOnChange>>)>
	TSimpleProperty(const InAssignType& InDefaultValue)
		: Super(InDefaultValue, FSimplePropertyOnChange::FDelegate())
	{
	}

	template<typename InAssignType
		UE_REQUIRES(std::negation_v<std::is_same<InAssignType, FSimplePropertyOnChange>>)>
	TSimpleProperty(InAssignType&& InDefaultValue)
		: Super(Forward<InAssignType>(InDefaultValue), FSimplePropertyOnChange::FDelegate())
	{
	}

	template<typename InAssignType
		UE_REQUIRES(std::negation_v<std::is_same<InAssignType, FSimplePropertyOnChange>>)>
	TSimpleProperty(const InAssignType& InDefaultValue, FSimplePropertyOnChange&& InUpdateFunc)
		: Super(InDefaultValue, Forward<FSimplePropertyOnChange::FDelegate>(InUpdateFunc.Callback))
	{
	}

	template<typename InAssignType
		UE_REQUIRES(std::negation_v<std::is_same<InAssignType, FSimplePropertyOnChange>>)>
	TSimpleProperty(InAssignType&& InDefaultValue, FSimplePropertyOnChange&& InUpdateFunc)
		: Super(Forward<InAssignType>(InDefaultValue), Forward<FSimplePropertyOnChange::FDelegate>(InUpdateFunc.Callback))
	{
	}

	template<typename InAssignType>
	bool operator=(const InAssignType& InValue)
	{
		return Super::SetInternal(InValue);
	}

	template<typename InAssignType>
	bool operator=(InAssignType&& InValue)
	{
		return Super::SetInternal(MoveTemp(InValue));
	}

	template<typename InAssignType>
	bool operator=(const TSimpleProperty<InAssignType>& InProperty)
	{
		return Super::SetInternal(InProperty.Value);
	}

	template<typename InAssignType>
	bool operator=(TSimpleProperty<InAssignType>&& InProperty)
	{
		return Super::SetInternal(MoveTemp(InProperty.Value));
	}

	operator FReferenceType&()
	{
		return FValueTypes::GetReferenceValue(Base::Value);
	}

	FReferenceType& operator*()
	{
		return FValueTypes::GetReferenceValue(Base::Value);
	}

	FPointerType* operator->()
	{
		return FValueTypes::GetPointerValue(Base::Value);
	}
};

template<typename InValueType>
struct TStructOpsTypeTraits<TSimplePropertyBase<InValueType>>
	: public TStructOpsTypeTraitsBase2<TSimplePropertyBase<InValueType>>
{
	enum
	{
		WithCopy = true,
		WithIdenticalViaEquality = true,
		WithAddStructReferencedObjects = TSimplePropertyTypeTraits<InValueType>::FReferenceCollectorType::Value,
		WithFindInnerPropertyInstance = TSimplePropertyTypeTraits<InValueType>::FFindInnerPropertyType::Value
	};
};
