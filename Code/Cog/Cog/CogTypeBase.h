#pragma once

class Program;

class CogTypeBase
{
public:
	using Base = CogTypeBase;

	CogTypeBase() = default;
	virtual ~CogTypeBase() = default;

	DELETE_COPYCONSTRUCTORS_AND_MOVES(CogTypeBase);

protected:
	virtual Program& GetProgram() const = 0;

	template <typename T>
	FORCEINLINE T& GetProgram() const
	{
		return CheckedCast<T>(GetProgram());
	}
};
