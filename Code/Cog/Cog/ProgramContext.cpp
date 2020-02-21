#include "CogPch.h"
#include "ProgramContext.h"
#include "TypeList.h"
#include "Program.h"

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
	myTypeIdToSingleton.Resize(TypeID<CogTypeBase>::MaxUnderlyingInteger() + 1);

	for (const TypeData* singletonType : gTypeList.GetSingletons())
	{
		Singleton& singleton = *mySingletonInstances.Add(singletonType->AllocateSingleton());

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
	{
		singleton->ConstructSingleton();
		singleton->SetOwningProgramContext(this);

		singleton->SetIsBaseCalled(false);
		singleton->Starting();
		// TODO: Update with type name
		CHECK_MSG(singleton->IsBaseCalled() == true, L"Singleton subclass did not call Base::Starting() all the way down to Singleton");
	}
}

void ProgramContext::DestroySingletons()
{
	for (Singleton* singleton : mySingletonInstances)
	{
		singleton->SetIsBaseCalled(false);
		singleton->ShuttingDown();
		// TODO: Update with type name
		CHECK_MSG(singleton->IsBaseCalled() == true, L"Singleton subclass did not call Base::ShuttingDown() all the way down to Singleton");

		singleton->DestructSingleton();
	}

	mySingletonInstances.Empty();
	myTypeIdToSingleton.Empty();
}
