#include <extdll.h>
#include <meta_api.h>

#include "interface.h"

#include <Angelscript/util/ASLogging.h>

#include "ASMod/IASEnvironment.h"

#include "StringInterop.h"

#include "CASSCInteropModule.h"

EXPOSE_SINGLE_INTERFACE( CASSCInteropModule, IASModModule, IASMODMODULE_NAME );

const char* CASSCInteropModule::GetName() const
{
	return "SCInterop";
}

const char* CASSCInteropModule::GetLogTag() const
{
	return "SC-INTEROP";
}

bool CASSCInteropModule::Initialize( const CreateInterfaceFn* pFactories, const size_t uiNumFactories, IASLogger* pLogger )
{
	if( !BaseClass::Initialize( pFactories, uiNumFactories, pLogger ) )
		return false;

	as::Diagnostic( "Registering Sven Co-op interop features\n" );

	auto& scriptEngine = *GetEnvironment().GetScriptEngine();

	RegisterScriptStringInterop( scriptEngine );

	return true;
}

bool CASSCInteropModule::Shutdown()
{
	return BaseClass::Shutdown();
}
