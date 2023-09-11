// Copyright Matt Chapman. All Rights Reserved.

#if WITH_EDITOR

#include "SimplePropertyTransactionManager.h"
#include "CoreGlobals.h"
#include "Engine/Engine.h"
#include "Internationalization/Text.h"

namespace UE::SimpleProperties::Private
{
	constexpr int32 InvalidTransactionId = INDEX_NONE;
	const TCHAR* SessionName = TEXT("SimpleProperties");
}

FSimplePropertyTransactionManager::FSimplePropertyTransactionManager()
{
	using namespace UE::SimpleProperties::Private;

	ManagedTransactionId = InvalidTransactionId;
}

FSimplePropertyTransactionManager& FSimplePropertyTransactionManager::Get()
{
	static FSimplePropertyTransactionManager Instance;
	return Instance;
}

int32 FSimplePropertyTransactionManager::GetInvalidTransactionId()
{
	using namespace UE::SimpleProperties::Private;

	return InvalidTransactionId;
}

bool FSimplePropertyTransactionManager::IsValidTransactionId(int32 InTransactionId)
{
	using namespace UE::SimpleProperties::Private;

	return InTransactionId != InvalidTransactionId && InTransactionId >= 0;
}

int32 FSimplePropertyTransactionManager::StartUnmanagedTransaction(const FText& InTransactionDescription, UObject* InPrimaryObject)
{
	using namespace UE::SimpleProperties::Private;

	// Uses the same checks as FScopedTransaction
	if (!GUndo || !GEngine || !GEngine->CanTransact() || !ensure(!GIsTransacting))
	{
		return InvalidTransactionId;
	}

	return GEditor->BeginTransaction(
		SessionName,
		InTransactionDescription, 
		InPrimaryObject
	);
}

TSharedPtr<FScopedTransaction> FSimplePropertyTransactionManager::StartUnmanagedScopedTransaction(
	const FText& InTransactionDescription, UObject* InPrimaryObject)
{
	using namespace UE::SimpleProperties::Private;

	TSharedRef<FScopedTransaction> NewTransaction = MakeShared<FScopedTransaction>(
		SessionName, 
		InTransactionDescription, 
		InPrimaryObject
	);

	if (!NewTransaction->IsOutstanding())
	{
		return nullptr;
	}

	return NewTransaction;
}

bool FSimplePropertyTransactionManager::IsManagedTransaction(int32 InTransactionId) const
{
	return ManagedTransactionId == InTransactionId && IsValidTransactionId(InTransactionId);
}

bool FSimplePropertyTransactionManager::IsUnmanagedTransaction(int32 InTransactionId) const
{
	return ManagedTransactionId != InTransactionId && IsValidTransactionId(InTransactionId);
}

bool FSimplePropertyTransactionManager::HasManagedTransaction() const
{
	return IsValidTransactionId(ManagedTransactionId);
}

int32 FSimplePropertyTransactionManager::StartManagedTransaction(const FText& InTransactionDescription, 
	UObject* InPrimaryObject)
{
	using namespace UE::SimpleProperties::Private;

	if (HasManagedTransaction())
	{
		return InvalidTransactionId;
	}

	const int32 NewTransactionId = StartUnmanagedTransaction(InTransactionDescription, InPrimaryObject);

	if (!IsValidTransactionId(NewTransactionId))
	{
		return InvalidTransactionId;
	}

	ManagedTransactionId = NewTransactionId;

	return NewTransactionId;
}

bool FSimplePropertyTransactionManager::CancelManagedTransaction(int32 InTransactionId)
{
	using namespace UE::SimpleProperties::Private;

	if (GEditor && ensure(IsManagedTransaction(InTransactionId)))
	{
		// No way of knowing if this was successful or not
		GEditor->CancelTransaction(ManagedTransactionId);
		ManagedTransactionId = InvalidTransactionId;
		return true;
	}

	return false;
}

bool FSimplePropertyTransactionManager::EndManagedTransaction(int32 InTransactionId)
{
	using namespace UE::SimpleProperties::Private;

	if (GEditor && ensure(IsManagedTransaction(InTransactionId)))
	{
		ensure(InTransactionId == GEditor->EndTransaction());
		ManagedTransactionId = InvalidTransactionId;
		return true;
	}

	return false;
}

bool FSimplePropertyTransactionManager::CancelUnmanagedTransaction(int32 InTransactionId)
{
	using namespace UE::SimpleProperties::Private;

	if (GEditor && ensure(IsUnmanagedTransaction(InTransactionId)))
	{
		// No way of knowing if this was successful or not
		GEditor->CancelTransaction(ManagedTransactionId);
		return true;
	}

	return false;
}

bool FSimplePropertyTransactionManager::EndUnmanagedTransaction(int32 InTransactionId)
{
	using namespace UE::SimpleProperties::Private;

	if (GEditor && ensure(IsUnmanagedTransaction(InTransactionId)))
	{
		ensure(InTransactionId == GEditor->EndTransaction());
		return true;
	}

	return false;
}

#endif
