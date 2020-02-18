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
	generatedLines.Add(Format(L"inline static const StringView StaticTypeName = L\"%\";", GetTypeName()));

	if (GetTypeName() != L"Object" && !myIsSingleton)
	{
		generatedLines.Add(String(L"void GetBaseClasses(const FunctionView<void(const TypeID<CogTypeBase>&)>& aFunction) const override"));
		generatedLines.Add(String(L"{ aFunction(TypeID<CogTypeBase>::Resolve<Base>()); Base::GetBaseClasses(aFunction); }"));
	}

	generatedLines.Add(String(L"private:"));

	if (!myIsSingleton)
	{
		generatedLines.Add(Format(L"FORCEINLINE const %CogTypeChunk& GetChunk() const { return static_cast<const %CogTypeChunk&>(*myChunk); }", GetTypeName(), GetTypeName()));
		generatedLines.Add(Format(L"FORCEINLINE %CogTypeChunk& GetChunk() { return static_cast<%CogTypeChunk&>(*myChunk); }", GetTypeName(), GetTypeName()));
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
			accessor = Format(L"_data_%", prop.propertyName);
			generatedLines.Add(String(L"private:"));
			generatedLines.Add(Format(L"% _data_%;", prop.propertyType, prop.propertyName));
		}

		if (prop.publicRead)
			generatedLines.Add(String(L"public:"));
		else
			generatedLines.Add(String(L"private:"));

		// Only generate the shorthand special name if we haven't specified DirectAccess in order to avoid a naming conflict
		if (!hasSpecialName || !prop.directAccess)
			generatedLines.Add(Format(L"FORCEINLINE ConstReferenceOf<%> %() const { return %; }", prop.propertyType, getterName, accessor));

		generatedLines.Add(String(L"private:"));

		if (prop.directAccess)
		{
			// This is mainly a template so we can SFINAE it out in case of it not being copy-constructible
			generatedLines.Add(Format(L"template<typename TCopyType, typename = EnableIf<std::is_copy_constructible_v<%>>> FORCEINLINE ReferenceOf<%> Set%(TCopyType aNewValue) { auto& value = %; value = Move(aNewValue); return value; }", prop.propertyType, prop.propertyType, prop.propertyName, accessor));
			
			generatedLines.Add(Format(L"FORCEINLINE ConstReferenceOf<%> %() const { return %; }", prop.propertyType, prop.propertyName, accessor));
			generatedLines.Add(Format(L"FORCEINLINE ReferenceOf<%> %() { return %; }", prop.propertyType, prop.propertyName, accessor));
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
			generatedLines.Add(Format(L"\tmemset(&my%Data[aIndex].Get(), 0, sizeof(%));", name, data.propertyType));
		

		generatedLines.Add(Format(L"\tmy%Data[aIndex].Construct(%);", name, data.defaultValue));
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

String CogClass::GenerateHeaderFileContents(const DocumentTemplates& aTemplates, const StringView aGeneratedHeaderIdentifier) const
{
	String headerOutput = Base::GenerateHeaderFileContents(aTemplates, aGeneratedHeaderIdentifier);

	{
		const Array<String> generatedBodyContents = GenerateGeneratedBodyContents(aGeneratedHeaderIdentifier);

		for (i32 i = 0; i < generatedBodyContents.GetLength(); ++i)
		{
			if (i > 0)
				headerOutput.Append(L" \\\n\t");

			headerOutput.Append(generatedBodyContents[i].View());
		}

		headerOutput.Add(L'\n');
	}

	if (!myIsSingleton)
	{
		const Array<String> chunkContents = GenerateCogTypeChunkHeaderContents();

		for (i32 i = 0; i < chunkContents.GetLength(); ++i)
		{
			if (i > 0)
				headerOutput.Add(L'\n');

			headerOutput.Append(chunkContents[i].View());
		}
	}

	return headerOutput;
}

String CogClass::GenerateSourceFileContents(const DocumentTemplates& aTemplates) const
{
	String sourceOutput = Base::GenerateSourceFileContents(aTemplates);

	if (!myIsSingleton)
	{
		const Array<String> chunkContents = GenerateCogTypeChunkSourceContents();

		for (i32 i = 0; i < chunkContents.GetLength(); ++i)
		{
			if (i > 0)
				sourceOutput.Add(L'\n');

			sourceOutput.Append(chunkContents[i].View());
		}
	}
	
	return sourceOutput;
}

void CogClass::SetIsFinal(const bool aIsFinal)
{
	myIsFinal = aIsFinal;
}

void CogClass::RegisterCogProperty(CogProperty aProperty)
{
	myProperties.Add(Move(aProperty));
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
		if (!prop.defaultValue.View() && !prop.zeroMemory)
			continue;

		ClassPropertyInitializerData& initializerData = aPropertyInitializers.FindOrAdd(prop.propertyName);
		initializerData.defaultValue = prop.defaultValue;
		initializerData.propertyType = prop.propertyType;
		initializerData.zeroMemory = prop.zeroMemory;
	}
}
