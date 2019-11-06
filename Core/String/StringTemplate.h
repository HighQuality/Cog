#pragma once

class StringTemplate
{
public:
	explicit StringTemplate(String aTemplate);
	
	void AddParameter(String aName, String aValue);
	void ClearParameters();

	String Evaluate() const;

private:
	Map<String, String> myParameters;
	String myTemplate;
};

