#include "CorePch.h"
#include "CommandLine.h"

void CommandLine::Parse(const ArrayView<StringView> aArguments)
{
	CHECK(aArguments.GetLength() > 0);
	myExecutablePath = String(aArguments[0]);

	for (i32 i = 1; i < aArguments.GetLength(); ++i)
	{
		const StringView argument = aArguments[i];

		if (argument.StartsWith(L"--"))
		{
			StringView value;

			if (i + 1 < aArguments.GetLength())
			{
				value = aArguments[i + 1];
				++i;
			}

			myParameters.Add(String(argument.RemoveFromStart(2)), String(value));
			continue;
		}

		myInput.Add(String(argument));
	}
}

i32 CommandLine::GetInteger(const StringView aParameterName, const i32 aDefaultValue) const
{
	i32 value;

	if (!TryGetInteger(aParameterName, value))
		value = aDefaultValue;

	return value;
}

bool CommandLine::TryGetInteger(const StringView aParameterName, i32& aValue) const
{
	StringView value;
	if (!TryGetString(aParameterName, value))
		return false;

	wchar_t* end;
	
	aValue = wcstol(value.GetData(), &end, 0);
	
	if (end == value.GetData())
		return false;

	return true;
}

bool CommandLine::GetBool(const StringView aParameterName, const bool aDefaultValue) const
{
	bool value;

	if (!TryGetBool(aParameterName, value))
		value = aDefaultValue;

	return value;
}

bool CommandLine::TryGetBool(StringView aParameterName, bool& aValue) const
{
	StringView value;
	if (!TryGetString(aParameterName, value))
		return false;

	if (value.GetLength() == 1)
	{
		switch (value[0])
		{
		case L'1':
			aValue = true;
			return true;
		case L'0':
			aValue = false;
			return true;
		}
	}

	const String lowerValue = value.ToLower();

	if (lowerValue == L"true")
	{
		aValue = true;
		return true;
	}
	else if(lowerValue == L"false")
	{
		aValue = false;
		return true;
	}

	return false;
}

StringView CommandLine::GetString(const StringView aParameterName, const StringView aDefaultValue) const
{
	StringView value;

	if (!TryGetString(aParameterName, value))
		value = aDefaultValue;

	return value;
}

bool CommandLine::TryGetString(StringView aParameterName, StringView& aValue) const
{
	if (const String* value = myParameters.Find(aParameterName))
	{
		aValue = *value;
		return true;
	}

	return false;
}

bool CommandLine::HasParameter(const StringView aParameterName) const
{
	return myParameters.ContainsKey(aParameterName);
}

bool CommandLine::ParameterHasValue(StringView aParameterName) const
{
	if (const String* value = myParameters.Find(aParameterName))
		return value->GetLength() > 0;
	return false;
}

void CommandLine::Parse(const i32 argc, const wchar_t* const* argv)
{
	Array<StringView> arguments;
	arguments.Resize(argc);

	for (i32 i = 0; i < argc; ++i)
		arguments[i] = StringView(argv[i]);

	Parse(arguments);
}

void CommandLine::Parse(const i32 argc, const char* const* argv)
{
	Array<String> arguments;
	arguments.Resize(argc);
	for (i32 i = 0; i < argc; ++i)
		arguments[i] = String(argv[i]);

	Array<StringView> argumentsView(arguments);
	Parse(argumentsView);
}
