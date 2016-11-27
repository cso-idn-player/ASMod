#include <extdll.h>
#include <meta_api.h>

#include "interface.h"

#include <Angelscript/util/IASLogger.h>
#include <Angelscript/wrapper/CASContext.h>
#include <Angelscript/ScriptAPI/SQL/CASSQLThreadPool.h>

#include "ASMod/IASEnvironment.h"

#include "MetaHelpers.h"

#include "Module.h"

#include "ASHLSQL.h"

#include "CASSQLModule.h"

EXPOSE_SINGLE_INTERFACE( CASSQLModule, IASModModule, IASMODMODULE_NAME );

const char* CASSQLModule::GetName() const
{
	return "SQL";
}

const char* CASSQLModule::GetLogTag() const
{
	return "SQL";
}

bool CASSQLModule::Initialize( const CreateInterfaceFn* pFactories, const size_t uiNumFactories, IASLogger* pLogger )
{
	if( !BaseClass::Initialize( pFactories, uiNumFactories, pLogger ) )
		return false;

	as_mysql_config = Meta_RegCVar( "as_mysql_config", "server/default_mysql_config.txt", FCVAR_SERVER | FCVAR_UNLOGGED );

	auto& scriptEngine = *m_pEnvironment->GetScriptEngine();

	RegisterScriptHLSQL( scriptEngine );

	return true;
}

bool CASSQLModule::Shutdown()
{
	g_pSQLThreadPool->Stop( false );

	return BaseClass::Shutdown();
}

void CASSQLModule::Think()
{
	CASOwningContext ctx( *GetEnvironment().GetScriptEngine() );
	g_pSQLThreadPool->ProcessQueue( *ctx.GetContext() );
}
