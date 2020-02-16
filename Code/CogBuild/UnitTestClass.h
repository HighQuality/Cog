#pragma once

struct UnitTest
{
	String functionName;
	String functionLocation;
};

struct UnitTestClass
{
	String className;
	Array<UnitTest> tests;
};
