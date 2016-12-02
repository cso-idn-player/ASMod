#ifndef ASMOD_CASMODBASEMODULE_H
#define ASMOD_CASMODBASEMODULE_H

#include "ASMod/IASModModule.h"

class IASMod;
class IASEnvironment;

/**
*	Base class for ASMod modules.
*/
class CASModBaseModule : public IASModModule
{
public:
	CASModBaseModule() = default;
	virtual ~CASModBaseModule() = default;

	bool Initialize( const CreateInterfaceFn* pFactories, const size_t uiNumFactories, IASLogger* pLogger ) override;

	bool Shutdown() override;

	/**
	*	@return The ASMod instance.
	*/
	IASMod& GetASMod() { return *m_pASMod; }

	/**
	*	@return The Angelscript environment.
	*/
	IASEnvironment& GetEnvironment() { return *m_pEnvironment; }

protected:
	IASMod* m_pASMod = nullptr;
	IASEnvironment* m_pEnvironment = nullptr;

private:
	CASModBaseModule( const CASModBaseModule& ) = delete;
	CASModBaseModule& operator=( const CASModBaseModule& ) = delete;
};

#endif //ASMOD_CASMODBASEMODULE_H
