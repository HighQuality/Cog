#include "CogBuildPch.h"
#include "CogClass.h"
#include "String/StringTemplate.h"
#include "DocumentTemplates.h"

CogClass::CogClass(String aAbsoluteFilePath, i32 aDeclarationLine, String aClassName, String aBaseClassName, i32 aGeneratedBodyLineIndex)
	: Base(Move(aAbsoluteFilePath), aDeclarationLine, Move(aClassName), Move(aBaseClassName))
{
	myGeneratedBodyLineIndex = aGeneratedBodyLineIndex;
	myChunkTypeName = Format(L"%CogTypeChunk", GetTypeName());
}

Array<String> CogClass::GenerateGeneratedBodyContents(const StringView aGeneratedHeaderIdentifier) const
{
	Array<String> generatedLines;

	generatedLines.Add(Format(L"#define GENERATED_BODY", aGeneratedHeaderIdentifier, myGeneratedBodyLineIndex + 1));
	generatedLines.Add(String(L"public:"));

	if (HasBaseType())
		generatedLines.Add(Format(L"using Base = %;", GetBaseTypeName()));
	else
		generatedLines.Add(Format(L"using Base = CogTypeBase;"));

	generatedLines.Add(Format(L"static constexpr bool StaticIsSpecialization = %;", mySpecializesBaseClass));
	generatedLines.Add(Format(L"static StringView GetStaticTypeName() { return L\"%\"; }", GetTypeName()));
	generatedLines.Add(String(L"const TypeData& GetType() const override;"));

	generatedLines.Add(String(L"private:"));

	if (!myIsSingleton)
	{
		generatedLines.Add(Format(L"FORCEINLINE const %CogTypeChunk& GetChunk() const { return static_cast<const %CogTypeChunk&>(*myChunk); }", GetTypeName(), GetTypeName()));
		generatedLines.Add(Format(L"FORCEINLINE %CogTypeChunk& GetChunk() { return static_cast<%CogTypeChunk&>(*myChunk); }", GetTypeName(), GetTypeName()));
		generatedLines.Add(Format(L"friend class %CogTypeChunk;", GetTypeName()));
	}
	
	generatedLines.Add(String(L"public:"));

	if (myIsSingleton)
	{
		const bool isBaseSingleton = GetTypeName() == L"Singleton";

		if (!isBaseSingleton)
		{
			generatedLines.Add(String(L"void ConstructSingleton() override;"));
			generatedLines.Add(String(L"void DestructSingleton() override;"));
		}
		else
		{
			generatedLines.Add(String(L"virtual void ConstructSingleton();"));
			generatedLines.Add(String(L"virtual void DestructSingleton();"));
		}
	}

	for (const CogProperty& prop : myProperties)
	{
		const bool hasSpecialName = prop.propertyName.StartsWith(L"Is");
		const String getterName = hasSpecialName ? String(prop.propertyName) : Format(L"Get%", prop.propertyName);

		String accessor;

		if (!myIsSingleton)
		{
			accessor = Format(L"GetChunk().Access%(myChunkIndex)", prop.propertyName);
		}
		else
		{
			accessor = Format(L"_data_%.Get()", prop.propertyName);
			generatedLines.Add(String(L"private:"));
			generatedLines.Add(Format(L"ManualInitializationObject<%> _data_%;", prop.propertyType, prop.propertyName));
		}

		if (prop.publicRead)
			generatedLines.Add(String(L"public:"));
		else
			generatedLines.Add(String(L"private:"));

		generatedLines.Add(Format(L"FORCEINLINE ConstReferenceOf<%> %() const { return %; }", prop.propertyType, getterName, accessor));

		generatedLines.Add(String(L"private:"));

		if (prop.directAccess)
		{
			// This is mainly a template so we can SFINAE it out in case of it not being copy-constructible
			generatedLines.Add(Format(L"template<typename TCopyType, typename = EnableIf<std::is_copy_constructible_v<%>>> FORCEINLINE ReferenceOf<%> Set%(TCopyType aNewValue) { auto& value = %; value = Move(aNewValue); return value; }", prop.propertyType, prop.propertyType, prop.propertyName, accessor));

			generatedLines.Add(Format(L"FORCEINLINE ReferenceOf<%> %() { return %; }", prop.propertyType, getterName, accessor));
		}
		else
		{
			generatedLines.Add(Format(L"FORCEINLINE ConstReferenceOf<%> Set%(% aNewValue) { auto& value = %; value = Move(aNewValue); return value; }", prop.propertyType, prop.propertyName, prop.propertyType, accessor));
		}
	}
	
	// Reset the default visibility of class
	generatedLines.Add(String(L"private:"));

	return generatedLines;
}

Array<String> CogClass::GenerateCogTypeChunkHeaderContents() const
{
	// Singletons don't have CogTypeChunks
	CHECK(!myIsSingleton);

	Array<String> generatedLines;

	const String baseChunkName = Format(L"%CogTypeChunk", HasBaseType() ? GetBaseTypeName() : L"");
	const StringView finalSpecifier = myIsFinal ? L"final " : L"";

	generatedLines.Add(Format(L"class % %: public %", myChunkTypeName, finalSpecifier, baseChunkName));
	generatedLines.Add(String(L"{"));
	generatedLines.Add(String(L"public:"));
	generatedLines.Add(Format(L"\tusing Base = %;", baseChunkName));

	generatedLines.Add(String(L"\tvoid GatherListeners(TypeMap<Event, EventBroadcastFunctionPtr>& aEventListeners, TypeMap<Impulse, ImpulseInvokerFunctionPtr>& aImpulseListeners) const override;"));

	generatedLines.Add(Format(L"\tUniquePtr<Object> CreateDefaultObject() const override;"));
	generatedLines.Add(Format(L"\tvoid InitializeObjectAtIndex(u8 aIndex) override;"));
	generatedLines.Add(Format(L"\tvoid DestructObjectAtIndex(u8 aIndex) override;"));
	
	generatedLines.Add(String(L"protected:"));
	
	for (const CogProperty& prop : myProperties)
		generatedLines.Add(Format(L"\tManualInitializationObject<%> my%Data[256];", prop.propertyType, prop.propertyName));

	generatedLines.Add(String(L"public:"));

	for (const CogProperty& prop : myProperties)
	{
		generatedLines.Add(Format(L"\tFORCEINLINE ConstReferenceOf<%> Access%(const u8 aIndex) const { return my%Data[aIndex].Get(); };", prop.propertyType, prop.propertyName, prop.propertyName));
		generatedLines.Add(Format(L"\tFORCEINLINE ReferenceOf<%> Access%(const u8 aIndex) { return my%Data[aIndex].Get(); };", prop.propertyType, prop.propertyName, prop.propertyName));
	}

	generatedLines.Add(String(L"};"));
	
	return generatedLines;
}

Array<String> CogClass::GenerateCogTypeChunkSourceContents() const
{
	// Singletons don't have CogTypeChunks
	CHECK(!myIsSingleton);

	Array<String> generatedLines;

	generatedLines.Add(String(L"template <typename TVoid = void>"));
	generatedLines.Add(Format(L"static void GatherListenersFor_%(TypeMap<Event, CogTypeChunk::EventBroadcastFunctionPtr>& aEventListeners, TypeMap<Impulse, CogTypeChunk::ImpulseInvokerFunctionPtr>& aImpulseListeners)", GetTypeName()));
	generatedLines.Add(String(L"{"));

	Array<CogListener> eventListeners;
	Array<CogListener> impulseListeners;

	GatherListeners(eventListeners, impulseListeners);

	for (const CogListener& listener : myEventListeners)
		generatedLines.Add(Format(L"\taEventListeners.FindOrAdd<%>() = [](const Event& aEvent, CogTypeChunk& aChunk, const ArrayView<u8> aIndices)\n\t{\n\t\t% obj;\n\t\tobj.myChunk = &aChunk;\n\t\tfor (const u8 index : aIndices)\n\t\t{\n\t\t\tobj.myChunkIndex = index;\n\t\t\tobj.%::%(static_cast<const %&>(aEvent));\n\t\t}\n\t};", listener.listenerType, GetTypeName(), GetTypeName(), listener.methodName, listener.listenerType));
	
	for (const CogListener& listener : myImpulseListeners)
		generatedLines.Add(Format(L"\taImpulseListeners.FindOrAdd<%>() = [](const Impulse& aImpulse, CogTypeChunk& aChunk, const u8 aIndex)\n\t{\n\t\t% obj;\n\t\tobj.myChunk = &aChunk;\n\t\tobj.myChunkIndex = aIndex;\n\t\tobj.%::%(static_cast<const %&>(aImpulse));\n\t};", listener.listenerType, GetTypeName(), GetTypeName(), listener.methodName, listener.listenerType));
	
	generatedLines.Add(String(L"}"));

	generatedLines.Add(Format(L"void %::GatherListeners(TypeMap<Event, EventBroadcastFunctionPtr>& aEventListeners, TypeMap<Impulse, ImpulseInvokerFunctionPtr>& aImpulseListeners) const", myChunkTypeName));
	generatedLines.Add(String(L"{"));
	generatedLines.Add(Format(L"\tif constexpr (!std::is_abstract_v<%>)", GetTypeName()));
	generatedLines.Add(Format(L"\t\tGatherListenersFor_%<>(aEventListeners, aImpulseListeners);", GetTypeName()));
	generatedLines.Add(String(L"}"));

	generatedLines.Add(Format(L"UniquePtr<Object> %::CreateDefaultObject() const", myChunkTypeName));
	generatedLines.Add(String(L"{"));
	generatedLines.Add(Format(L"\tif constexpr (!std::is_abstract_v<%>)", GetTypeName()));
	generatedLines.Add(Format(L"\t\treturn MakeUnique<%>();", GetTypeName()));
	generatedLines.Add(String(L"\tFATAL(L\"Can't instantiate object of abstract type\");"));
	generatedLines.Add(String(L"}"));

	Map<StringView, ClassPropertyInitializerData> allProperties;
	GatherPropertyInitializers(allProperties);

	generatedLines.Add(Format(L"void %::InitializeObjectAtIndex(const u8 aIndex)", myChunkTypeName));
	generatedLines.Add(String(L"{"));
	generatedLines.Add(Format(L"\tif constexpr (std::is_abstract_v<%>)", GetTypeName()));
	generatedLines.Add(String(L"\t\treturn;"));

	for (const KeyValuePair<StringView, ClassPropertyInitializerData>& propertyPair : allProperties)
	{
		const StringView name = propertyPair.key;
		const ClassPropertyInitializerData data = propertyPair.value;

		if (data.zeroMemory)
			generatedLines.Add(Format(L"\tmy%Data[aIndex].ZeroData();", name));
		
		generatedLines.Add(Format(L"\tmy%Data[aIndex].Construct(%);", name, data.constructionArguments));

		if (data.defaultAssignment)
			generatedLines.Add(Format(L"\tmy%Data[aIndex].Get() = %;", name, data.defaultAssignment));

		generatedLines.Add(String(L""));
	}

	generatedLines.Add(String(L"}"));

	generatedLines.Add(Format(L"void %::DestructObjectAtIndex(const u8 aIndex)", myChunkTypeName));
	generatedLines.Add(String(L"{"));
	generatedLines.Add(Format(L"\tif constexpr (std::is_abstract_v<%>)", GetTypeName()));
	generatedLines.Add(String(L"\t\treturn;"));

	for (const KeyValuePair<StringView, ClassPropertyInitializerData>& propertyPair : allProperties)
	{
		const StringView name = propertyPair.key;
		const ClassPropertyInitializerData data = propertyPair.value;

		generatedLines.Add(Format(L"\tmy%Data[aIndex].Destruct();", name));
	}

	generatedLines.Add(String(L"}"));

	return generatedLines;
}

Array<String> CogClass::GenerateHeaderFileContents(const DocumentTemplates& aTemplates, const StringView aGeneratedHeaderIdentifier) const
{
	Array<String> headerOutput = Base::GenerateHeaderFileContents(aTemplates, aGeneratedHeaderIdentifier);

	{
		const Array<String> generatedBodyContents = GenerateGeneratedBodyContents(aGeneratedHeaderIdentifier);

		for (i32 i = 0; i < generatedBodyContents.GetLength(); ++i)
		{
			if (i + 1 == generatedBodyContents.GetLength())
				headerOutput.Add(Format(L"\t%", generatedBodyContents[i]));
			else if (i > 0)
				headerOutput.Add(Format(L"\t%\\", generatedBodyContents[i]));
			else 
				headerOutput.Add(Format(L"%\\", generatedBodyContents[i]));
		}
	}

	if (!myIsSingleton)
	{
		const Array<String> chunkContents = GenerateCogTypeChunkHeaderContents();
		headerOutput.Append(chunkContents);
	}

	return headerOutput;
}

Array<String> CogClass::GenerateSourceFileContents(const DocumentTemplates& aTemplates) const
{
	Array<String> sourceOutput = Base::GenerateSourceFileContents(aTemplates);

	if (!myIsSingleton)
	{
		const Array<String> chunkContents = GenerateCogTypeChunkSourceContents();
		sourceOutput.Append(chunkContents);
	}

	sourceOutput.Add(Format(L"const TypeData& %::GetType() const", GetTypeName()));
	sourceOutput.Add(String(L"{"));
	sourceOutput.Add(String(L"\tstatic const TypeData& data = gTypeList.GetTypeData(GetStaticTypeName(), false);"));
	sourceOutput.Add(String(L"\treturn data;"));
	sourceOutput.Add(String(L"}"));

	if (myIsSingleton)
		sourceOutput.Append(GenerateSingletonInitialization());

	return sourceOutput;
}

Array<String> CogClass::GenerateSingletonInitialization() const
{
	CHECK(myIsSingleton);

	Array<String> generatedLines;
	Array<String> destructFunc;
	const bool isBaseSingleton = GetTypeName() == L"Singleton";

	if (!isBaseSingleton)
	{
		generatedLines.Add(Format(L"void %::ConstructSingleton()", GetTypeName()));
		generatedLines.Add(String(L"{"));
		generatedLines.Add(String(L"\tBase::ConstructSingleton();"));

		destructFunc.Add(Format(L"void %::DestructSingleton()", GetTypeName()));
		destructFunc.Add(String(L"{"));
	}
	else
	{
		generatedLines.Add(Format(L"void %::ConstructSingleton()", GetTypeName()));
		generatedLines.Add(String(L"{"));

		destructFunc.Add(Format(L"void %::DestructSingleton()", GetTypeName()));
		destructFunc.Add(String(L"{"));
	}

	for (const CogProperty& prop : myProperties)
	{
		if (prop.zeroMemory)
			generatedLines.Add(Format(L"\t_data_%.ZeroData();", prop.propertyName));

		generatedLines.Add(Format(L"\t_data_%.Construct(%);", prop.propertyName, prop.constructionArguments));

		if (prop.defaultAssignment.View())
			generatedLines.Add(Format(L"\t_data_%.Get() = %;", prop.propertyName, prop.defaultAssignment));

		destructFunc.Add(Format(L"\t_data_%.Destruct();", prop.propertyName));
	}

	if (!isBaseSingleton)
		destructFunc.Add(String(L"\tBase::DestructSingleton();"));

	generatedLines.Add(String(L"}"));
	destructFunc.Add(String(L"}"));

	generatedLines.Append(destructFunc);
	return generatedLines;
}

void CogClass::SetIsFinal(const bool aIsFinal)
{
	myIsFinal = aIsFinal;
}

void CogClass::RegisterCogProperty(CogProperty aProperty)
{
	myProperties.Add(Move(aProperty));
}

void CogClass::RegisterEventListener(CogListener aListener)
{
	myEventListeners.Add(Move(aListener));
}

void CogClass::RegisterImpulseListener(CogListener aListener)
{
	myImpulseListeners.Add(Move(aListener));
}

void CogClass::PostResolveDependencies()
{
	const CogClass* root = GetRootClass();
	CHECK(root);
	if (root->GetTypeName() == L"Singleton")
		myIsSingleton = true;
	else if (root->GetTypeName() == L"Object")
		myIsSingleton = false;
	else
		FATAL(L"Unknown root class ", root->GetTypeName());
}

const CogClass* CogClass::GetRootClass() const
{
	if (const CogClass* base = GetBaseType())
		return base->GetRootClass();
	return this;
}

void CogClass::GatherPropertyInitializers(Map<StringView, ClassPropertyInitializerData>& aPropertyInitializers) const
{
	if (const CogClass* baseType = GetBaseType())
		baseType->GatherPropertyInitializers(aPropertyInitializers);

	for (const CogProperty& prop : myProperties)
	{
		ClassPropertyInitializerData& initializerData = aPropertyInitializers.FindOrAdd(prop.propertyName);
		initializerData.constructionArguments = prop.constructionArguments;
		initializerData.defaultAssignment = prop.defaultAssignment;
		initializerData.propertyType = prop.propertyType;
		initializerData.zeroMemory = prop.zeroMemory;
	}
}

void CogClass::GatherListeners(Array<CogListener>& aEventListeners, Array<CogListener>& aImpulseListeners) const
{
	if (const CogClass* base = GetBaseType())
		base->GatherListeners(aEventListeners, aImpulseListeners);

	aEventListeners.Append(myEventListeners);
	aImpulseListeners.Append(myImpulseListeners);
}
