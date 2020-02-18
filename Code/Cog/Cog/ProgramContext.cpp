#include "CogPch.h"
#include "ProgramContext.h"
#include "TypeList.h"
#include "Program.h"

extern UniquePtr<const TypeList> gTypeList;

ProgramContext::ProgramContext() = default;
ProgramContext::~ProgramContext() = default;

void ProgramContext::Run()
{
	InitializeSingletons();

	Program& program = GetSingleton<Program>();
	program.Run();

	DestroySingletons();
}

void ProgramContext::InitializeSingletons()
{
	CHECK(gTypeList.IsValid());

	myTypeIdToSingleton.Resize(TypeID<CogTypeBase>::MaxUnderlyingInteger() + 1);

	for (const TypeData* singletonType : gTypeList->GetSingletons())
	{
		Singleton& singleton = *mySingletonInstances.Add(singletonType->AllocateSingleton());
		singleton.SetOwningProgramContext(this);

		const TypeData* currentType = singletonType;

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
	}

	for (Singleton* singleton : mySingletonInstances)
		singleton->Starting();
}

void ProgramContext::DestroySingletons()
{
	for (Singleton* singleton : mySingletonInstances)
		singleton->ShuttingDown();

	mySingletonInstances.Empty();
	myTypeIdToSingleton.Empty();
}
