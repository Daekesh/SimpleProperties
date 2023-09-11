// Copyright Matt Chapman. All Rights Reserved.

#pragma once

#include "Concepts/EqualityComparable.h"
#include "SimplePropertyConcepts.h"
#include "Templates/Models.h"
#include "Templates/SharedPointer.h"
#include "Templates/UnrealTypeTraits.h"
#include "UObject/ObjectPtr.h"

class FReferenceCollector;
class UObject;

template<typename InValueType>
struct TSimplePropertyValueTypes
{
	using FValueType = InValueType;
	using FReferenceType = FValueType;
	using FPointerType = FValueType;

	static const FReferenceType& GetReferenceValue(const InValueType& InValue)
	{
		return InValue;
	}

	static FReferenceType& GetReferenceValue(InValueType& InValue)
	{
		return InValue;
	}

	static const FPointerType* GetPointerValue(const InValueType& InValue)
	{
		return &InValue;
	}

	static FPointerType* GetPointerValue(InValueType& InValue)
	{
		return &InValue;
	}
};

struct TSimplePropertyComparator
{
	template<typename InValueType, typename InCompareType>
	static bool IsEqual(const InValueType& InValue, const InCompareType& InOther)
	{
		if constexpr (TModels<CEqualityComparableWith, InValueType, InCompareType>::Value)
		{
			return InValue == InOther;
		}
		if constexpr (TModels<CEqualityEquals, InValueType, InCompareType>::Value)
		{
			return InValue.Equals(InOther);
		}
		if constexpr (TModels<CEqualityEqualTo, InValueType, InCompareType>::Value)
		{
			return InValue.EqualTo(InOther);
		}

		return false;
	}
};

template<typename InValueType>
struct TSimplePropertyReferenceCollector
{
	enum
	{
		Struct = TModels<CCanCollectReferences, InValueType>::Value,
		Object = TIsDerivedFrom<typename TRemovePointer<InValueType>::Type, UObject>::Value,
		Value = Struct || Object
	};

	template<typename InValueType
		UE_REQUIRES(Struct)>
	static bool AddReferences(const InValueType& InValue, FReferenceCollector& InCollector)
	{
		return InValue->AddStructReferencedObjects(InCollector);
	}

	template<typename InValueType
		UE_REQUIRES(Object)>
	static bool AddReferences(const InValueType& InValue, FReferenceCollector& InCollector)
	{
		UObject* Value = InValue;

		if (IsValid(Value))
		{
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
			InCollector.AddReferencedObject(Value);
			PRAGMA_ENABLE_DEPRECATION_WARNINGS

			return true;
		}

		return false;
	}
};

template<typename InValueType>
struct TSimplePropertyInnerPropertyFinder
{
	enum { Value = TModels<CCanFindInnerPropertyInstance, InValueType>::Value };

	template<typename InValueType
		UE_REQUIRES(Value)>
	static bool FindInnerPropertyInstance(const InValueType& InValue, FName InPropertyName, const FProperty*& OutProp, const void*& OutData)
	{
		return InValue->FindInnerPropertyInstance(InPropertyName, OutProp, OutData);
	}
};

template<typename InValueType>
struct TSimplePropertyTransactionObject
{
	using IsPointerType = TIsPointer<InValueType>;
	using IsUObjectPtr = TIsDerivedFrom<typename TRemovePointer<InValueType>::Type, UObject>;

	static UObject* GetTransactionObject(const InValueType& InValue)
	{
		if constexpr(TAnd<IsPointerType, IsUObjectPtr>::Value)
		{
			if (IsValid(InValue))
			{
				return InValue;
			}
		}

		return nullptr;
	}
};

template<typename InValueType>
struct TSimplePropertyTypeTraits
{
	using FValueTypes = TSimplePropertyValueTypes<InValueType>;
	using FComparatorType = TSimplePropertyComparator;
	using FReferenceCollectorType = TSimplePropertyReferenceCollector<InValueType>;
	using FFindInnerPropertyType = TSimplePropertyInnerPropertyFinder<InValueType>;
	using FTransactionObjectType = TSimplePropertyTransactionObject<InValueType>;
};

template<typename InValueType>
struct TSimplePropertyReferenceCollector<TObjectPtr<InValueType>>
{
	using FValueType = TObjectPtr<InValueType>;

	static bool AddReferences(const FValueType& InValue, FReferenceCollector& InCollector)
	{
		if (IsValid(InValue.Get()))
		{
			FValueType ObjectPtr = InValue;
			InCollector.AddReferencedObject(ObjectPtr);
			return true;
		}

		return false;
	}
};

template<typename InValueType>
struct TSimplePropertyValueTypes<TObjectPtr<InValueType>>
{
	using FValueType = TObjectPtr<InValueType>;
	using FReferenceType = FValueType*;
	using FPointerType = InValueType;

	static const FReferenceType& GetReferenceValue(const FValueType& InValue)
	{
		return InValue;
	}

	static FReferenceType& GetReferenceValue(FValueType& InValue)
	{
		return InValue;
	}

	static const FPointerType* GetPointerValue(const FValueType& InValue)
	{
		return &InValue;
	}

	static FPointerType* GetPointerValue(FValueType& InValue)
	{
		return &InValue;
	}
};

template<typename InValueType>
struct TSimplePropertyTransactionObject<TObjectPtr<InValueType>>
{
	using FValueType = TObjectPtr<InValueType>;

	static UObject* GetTransactionObject(const FValueType& InValue)
	{
		if (UObject* Object = InValue.Get(); IsValid(Object))
		{
			return Object;
		}

		return nullptr;
	}
};

template<typename InValueType>
struct TSimplePropertyValueTypes<TWeakPtr<InValueType>>
{
	using FValueType = TWeakPtr<InValueType>;
	using FReferenceType = FValueType;
	using FPointerType = InValueType;

	static const FReferenceType& GetReferenceValue(const FValueType& InValue)
	{
		return InValue;
	}

	static FReferenceType& GetReferenceValue(FValueType& InValue)
	{
		return InValue;
	}

	static const FPointerType* GetPointerValue(const FValueType& InValue)
	{
		return InValue.Pin().Get();
	}

	static FPointerType* GetPointerValue(FValueType& InValue)
	{
		return InValue.Pin().Get();
	}
};

template<typename InValueType>
struct TSimplePropertyValueTypes<TSharedPtr<InValueType>>
{
	using FValueType = TSharedPtr<InValueType>;
	using FReferenceType = FValueType;
	using FPointerType = InValueType;

	static const FReferenceType& GetReferenceValue(const FValueType& InValue)
	{
		return InValue;
	}

	static FReferenceType& GetReferenceValue(FValueType& InValue)
	{
		return InValue;
	}

	static const FPointerType* GetPointerValue(const FValueType& InValue)
	{
		return InValue.Get();
	}

	static FPointerType* GetPointerValue(FValueType& InValue)
	{
		return InValue.Get();
	}
};

template<typename InValueType>
struct TSimplePropertyValueTypes<TSharedRef<InValueType>>
{
	using FValueType = TSharedRef<InValueType>;
	using FReferenceType = FValueType;
	using FPointerType = InValueType;

	static const FReferenceType& GetReferenceValue(const FValueType& InValue)
	{
		return InValue;
	}

	static FReferenceType& GetReferenceValue(FValueType& InValue)
	{
		return InValue;
	}

	static const FPointerType* GetPointerValue(const FValueType& InValue)
	{
		return &*InValue;
	}

	static FPointerType* GetPointerValue(FValueType& InValue)
	{
		return &*InValue;
	}
};
