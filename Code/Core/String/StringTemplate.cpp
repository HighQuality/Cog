#include "CorePch.h"
#include "StringTemplate.h"

StringTemplate::StringTemplate(String aTemplate)
{
	myTemplate = Move(aTemplate);
}

void StringTemplate::AddParameter(String aName, String aValue)
{
	myParameters.Add(Move(aName), Move(aValue));
}

void StringTemplate::ClearParameters()
{
	myParameters.Clear();
}

String StringTemplate::Evaluate() const
{
	String result;
	const i32 length = myTemplate.GetLength();
	i32 previousIndex = 0;

	for (i32 i = 0; i < length; ++i)
	{
		if (myTemplate[i] == L'$')
		{
			if (myTemplate.TryGet(i + 1, L'\0') == L'{')
			{
				result.Append(myTemplate.Slice(previousIndex, i - previousIndex));
				i += 2;
				previousIndex = i;

				bool foundEnd = false;

				while (const Char character = myTemplate.TryGet(i, L'\0'))
				{
					if (character == L'}')
					{
						foundEnd = true;
						break;
					}

					++i;
				}

				if (!ENSURE(foundEnd))
					return String();

				const StringView parameter = myTemplate.Slice(previousIndex, i - previousIndex);
				const String* parameterValue = myParameters.Find(parameter);

				if (ENSURE_MSG(parameterValue, L"Parameter \"%\" on string template not found", parameter))
				{
					const StringView value = *parameterValue;
					result.Append(value);
				}

				previousIndex = i + 1;
			}
		}
	}

	result.Append(myTemplate.Slice(previousIndex, length - previousIndex));

	return result;
}
