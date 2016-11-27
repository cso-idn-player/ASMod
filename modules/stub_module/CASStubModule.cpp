#include <extdll.h>
#include <meta_api.h>

#include "interface.h"

#include <Angelscript/util/ASLogging.h>

#include "CASStubModule.h"

EXPOSE_SINGLE_INTERFACE( CASStubModule, IASModModule, IASMODMODULE_NAME );

const char* CASStubModule::GetName() const
{
	return "stub_module";
}

bool CASStubModule::Initialize( const CreateInterfaceFn* pFactories, const size_t uiNumFactories )
{
	if( !BaseClass::Initialize( pFactories, uiNumFactories ) )
		return false;

	as::Diagnostic( "Initializing module\n" );

	return true;
}

bool CASStubModule::Shutdown()
{
	as::Diagnostic( "Shutting down module\n" );

	return BaseClass::Shutdown();
}
