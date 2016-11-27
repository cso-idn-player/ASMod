#ifndef SCINTEROP_CASSCINTEROPMODULE_H
#define SCINTEROP_CASSCINTEROPMODULE_H

#include "ASMod/Module/CASModBaseModule.h"

class CASSCInteropModule : public CASModBaseModule
{
public:
	typedef CASSCInteropModule ThisClass;
	typedef CASModBaseModule BaseClass;

public:
	CASSCInteropModule() = default;
	~CASSCInteropModule() = default;

	const char* GetName() const override final;

	const char* GetLogTag() const override final;

	bool Initialize( const CreateInterfaceFn* pFactories, const size_t uiNumFactories, IASLogger* pLogger ) override;

	bool Shutdown() override;

private:
	CASSCInteropModule( const CASSCInteropModule& ) = delete;
	CASSCInteropModule& operator=( const CASSCInteropModule& ) = delete;
};

#endif //SCINTEROP_CASSCINTEROPMODULE_H
