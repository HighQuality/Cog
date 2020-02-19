#pragma once

class CommandLine
{
public:
	static CommandLine& Get()
	{
		static CommandLine ourInstance;
		return ourInstance;
	}

	void Parse(i32 argc, const wchar_t* const* argv);
	void Parse(i32 argc, const char* const* argv);
	void Parse(ArrayView<StringView> aArguments);

	i32 GetInteger(StringView aParameterName, i32 aDefaultValue) const;
	bool TryGetInteger(StringView aParameterName, i32& aValue) const;

	bool GetBool(StringView aParameterName, bool aDefaultValue) const;
	bool TryGetBool(StringView aParameterName, bool& aValue) const;

	StringView GetString(StringView aParameterName, StringView aDefaultValue) const;
	bool TryGetString(StringView aParameterName, StringView& aValue) const;

	bool HasParameter(StringView aParameterName) const;
	bool ParameterHasValue(StringView aParameterName) const;

	StringView GetExecutablePath() const { return myExecutablePath; }

private:
	CommandLine() = default;
	~CommandLine() = default;

	Map<String, String> myParameters;
	Array<String> myInput;
	String myExecutablePath;
};
