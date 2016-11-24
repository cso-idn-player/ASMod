#include <extdll.h>
#include <meta_api.h>

#include "interface.h"

#include <Angelscript/util/IASLogger.h>

#include "ASMod/IASEnvironment.h"

#include "CASFileSystemModule.h"

EXPOSE_SINGLE_INTERFACE( CASFileSystemModule, IASModModule, IASMODMODULE_NAME );

const char* CASFileSystemModule::GetName() const
{
	return "stub_module";
}

bool CASFileSystemModule::Initialize( const CreateInterfaceFn* pFactories, const size_t uiNumFactories )
{
	if( !BaseClass::Initialize( pFactories, uiNumFactories ) )
		return false;

	return true;
}

bool CASFileSystemModule::Shutdown()
{
	return BaseClass::Shutdown();
}
