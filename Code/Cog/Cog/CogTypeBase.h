#pragma once

class Program;
class Singleton;
class ProgramContext;
class TypeData;

class CogTypeBase
{
public:
	using Base = CogTypeBase;
	inline static const StringView StaticTypeName = L"CogTypeBase";
	static constexpr bool StaticIsSpecialization = false;

	CogTypeBase() = default;
	virtual ~CogTypeBase() = default;

	DELETE_COPYCONSTRUCTORS_AND_MOVES(CogTypeBase);

protected:
	template <typename T>
	FORCEINLINE T& GetSingleton() const
	{
		return reinterpret_cast<T&>(GetSingleton(TypeID<CogTypeBase>::Resolve<T>()));
	}

	template <typename T = Program>
	FORCEINLINE T& GetProgram() const
	{
		return GetSingleton<T>();
	}

	Singleton& GetSingleton(const TypeID<CogTypeBase>& aTypeId) const;

	virtual ProgramContext& GetProgramContext() const = 0;
	virtual const TypeData& GetType() const = 0;
};
