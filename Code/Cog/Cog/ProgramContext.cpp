#include "CogPch.h"
#include "ProgramContext.h"
#include "TypeList.h"

extern UniquePtr<const TypeList> gTypeList;

ProgramContext::ProgramContext() = default;
ProgramContext::~ProgramContext() = default;

void ProgramContext::Run()
{
	InitializeSingletons();
}

Singleton& ProgramContext::ResolveSingleton(const TypeID<CogTypeBase>& aTypeId)
{
	CHECK(myIsInitializingSingletons);

	if (Singleton* singleton = myTypeIdToSingleton.TryGet(aTypeId.GetUnderlyingInteger(), nullptr))
		return *singleton;

	const TypeData& typeData = gTypeList->GetTypeData(aTypeId);

	CHECK(typeData.IsSingleton());

	Singleton& singleton = *mySingletonInstances.Add(typeData.AllocateSingleton());
	singleton.SetOwningProgramContext(this);

	const TypeData* currentType = &typeData;

	while (currentType)
	{
		const u16 typeIndex = currentType->GetTypeID().GetUnderlyingInteger();
		// This is only here as a precaution, types specializing an already specialized type should be caught earlier when building the type list
		CHECK(!myTypeIdToSingleton[typeIndex]);
		myTypeIdToSingleton[typeIndex] = &singleton;

		if (currentType->IsSpecializingBaseType())
			currentType = currentType->GetBaseType();
		else
			break;
	}

	return singleton;
}

void ProgramContext::InitializeSingletons()
{
	// TODO: A macro for this would be nice
	myIsInitializingSingletons = true;
	defer { myIsInitializingSingletons = false; };

	CHECK(gTypeList.IsValid());

	myTypeIdToSingleton.Resize(TypeID<CogTypeBase>::MaxUnderlyingInteger() + 1);

	for (const TypeData* singletonType : gTypeList->GetSingletons())
		ResolveSingleton(singletonType->GetTypeID());

	for (Singleton* singleton : mySingletonInstances)
		singleton->Starting();
}
