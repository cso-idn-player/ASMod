#ifndef ASMOD_CASSIMPLEENVIRONMENT_H
#define ASMOD_CASSIMPLEENVIRONMENT_H

#include <Angelscript/util/CASRefPtr.h>

#include "ASMod/IASEnvironment.h"

/**
*	A simple environment where all environment members are class members.
*/
class CASSimpleEnvironment : public IASEnvironment
{
public:
	CASSimpleEnvironment() = default;

	/**
	*	Constructs a new environment that is initialized to the given members.
	*/
	CASSimpleEnvironment( asIScriptEngine* pScriptEngine,
						  asGETCONTEXTFN_t contextFunc,
						  asALLOCFUNC_t allocFunc, asFREEFUNC_t freeFunc,
						  asALLOCFUNC_t arrayAllocFunc, asFREEFUNC_t arrayFreeFunc,
						  IASLogger* pLogger = nullptr );

	/**
	*	Copies the given environment into this one.
	*/
	CASSimpleEnvironment( IASEnvironment& environment );

	CASSimpleEnvironment( const CASSimpleEnvironment& other ) = default;
	CASSimpleEnvironment& operator=( const CASSimpleEnvironment& other ) = default;
	CASSimpleEnvironment( CASSimpleEnvironment&& other ) = default;
	CASSimpleEnvironment& operator=( CASSimpleEnvironment&& other ) = default;

	asPWORD GetInfo( const ASEnvInfo::ASEnvInfo info ) override;

	asIScriptEngine* GetScriptEngine() override final { return m_ScriptEngine.Get(); }

	asGETCONTEXTFN_t GetContextFunc() override final { return m_ContextFunc; }

	asGETLIBVERSIONFN GetLibVersionFunc() override final { return m_LibVersionFunc; }

	int GetLibVersion() override final { return m_iLibVersion; }

	asGETLIBOPTIONSFN GetLibOptionsFunc() override final { return m_LibOptionsFunc; }

	asATOMICINCFN GetAtomicIncFunc() override final { return m_AtomicIncFunc; }

	asATOMICDECFN GetAtomicDecFunc() override final { return m_AtomicDecFunc; }

	asALLOCFUNC_t GetAllocFunc() override final { return m_AllocFunc; }

	asFREEFUNC_t GetFreeFunc() override final { return m_FreeFunc; }

	asALLOCFUNC_t GetArrayAllocFunc() override final { return m_ArrayAllocFunc; }

	asFREEFUNC_t GetArrayFreeFunc() override final { return m_ArrayFreeFunc; }

	IASLogger* GetLogger() override final { return m_Logger; }

	void SetScriptEngine( asIScriptEngine* pScriptEngine )
	{
		m_ScriptEngine = pScriptEngine;
	}

	void SetContextFunc( asGETCONTEXTFN_t contextFunc )
	{
		m_ContextFunc = contextFunc;
	}

	void SetLibVersionFunc( asGETLIBVERSIONFN libVersionFunc )
	{
		m_LibVersionFunc = libVersionFunc;
	}

	void SetLibVersion( int iLibVersion )
	{
		m_iLibVersion = iLibVersion;
	}

	void SetLibOptionsFunc( asGETLIBOPTIONSFN libOptionsFunc )
	{
		m_LibOptionsFunc = libOptionsFunc;
	}

	void SetAtomicIncFunc( asATOMICINCFN atomicIncFunc )
	{
		m_AtomicIncFunc = atomicIncFunc;
	}

	void SetAtomicDecFunc( asATOMICDECFN atomicDecFunc )
	{
		m_AtomicDecFunc = atomicDecFunc;
	}

	void SetLogger( IASLogger* pLogger )
	{
		m_Logger = pLogger;
	}

	void SetAllocFunc( asALLOCFUNC_t allocFunc )
	{
		m_AllocFunc = allocFunc;
	}

	void SetFreeFunc( asFREEFUNC_t freeFunc )
	{
		m_FreeFunc = freeFunc;
	}

	void SetArrayAllocFunc( asALLOCFUNC_t allocFunc )
	{
		m_ArrayAllocFunc = allocFunc;
	}

	void SetArrayFreeFunc( asFREEFUNC_t freeFunc )
	{
		m_ArrayFreeFunc = freeFunc;
	}

	/**
	*	@return Whether this environment is valid. An environment is valid if all required members are set.
	*/
	bool IsValid() const
	{
		return 
			m_ScriptEngine.Get() && 
			m_ContextFunc && 
			m_LibVersionFunc &&
			m_iLibVersion != 0 &&
			m_LibOptionsFunc &&
			m_AtomicIncFunc &&
			m_AtomicDecFunc &&
			m_AllocFunc && 
			m_FreeFunc && 
			m_ArrayAllocFunc && 
			m_ArrayFreeFunc;
	}

private:
	CASRefPtr<asIScriptEngine> m_ScriptEngine;

	asGETCONTEXTFN_t m_ContextFunc = nullptr;

	asGETLIBVERSIONFN m_LibVersionFunc = asGetLibraryVersion;
	int m_iLibVersion = ANGELSCRIPT_VERSION;
	asGETLIBOPTIONSFN m_LibOptionsFunc = asGetLibraryOptions;

	asATOMICINCFN m_AtomicIncFunc = asAtomicInc;
	asATOMICDECFN m_AtomicDecFunc = asAtomicDec;

	asALLOCFUNC_t m_AllocFunc = nullptr;
	asFREEFUNC_t m_FreeFunc = nullptr;
	asALLOCFUNC_t m_ArrayAllocFunc = nullptr;
	asFREEFUNC_t m_ArrayFreeFunc = nullptr;

	CASRefPtr<IASLogger> m_Logger;
};

inline CASSimpleEnvironment::CASSimpleEnvironment( asIScriptEngine* pScriptEngine, 
												   asGETCONTEXTFN_t contextFunc,
												   asALLOCFUNC_t allocFunc, asFREEFUNC_t freeFunc, 
												   asALLOCFUNC_t arrayAllocFunc, asFREEFUNC_t arrayFreeFunc,
												   IASLogger* pLogger )
	: m_ScriptEngine( pScriptEngine )
	, m_ContextFunc( contextFunc )
	, m_AllocFunc( allocFunc )
	, m_FreeFunc( freeFunc )
	, m_ArrayAllocFunc( arrayAllocFunc )
	, m_ArrayFreeFunc( arrayFreeFunc )
	, m_Logger( pLogger )
{
}

inline CASSimpleEnvironment::CASSimpleEnvironment( IASEnvironment& environment )
	: m_ScriptEngine( environment.GetScriptEngine() )
	, m_ContextFunc( environment.GetContextFunc() )
	, m_AllocFunc( environment.GetAllocFunc() )
	, m_FreeFunc( environment.GetFreeFunc() )
	, m_ArrayAllocFunc( environment.GetArrayAllocFunc() )
	, m_ArrayFreeFunc( environment.GetArrayFreeFunc() )
	, m_Logger( environment.GetLogger() )
{
}

inline asPWORD CASSimpleEnvironment::GetInfo( const ASEnvInfo::ASEnvInfo info )
{
	switch( info )
	{
	case ASEnvInfo::LIBRARY_VERSION_STRING:	return reinterpret_cast<asPWORD>( m_LibVersionFunc() );
	case ASEnvInfo::LIBRARY_VERSION_INT:	return m_iLibVersion;
	case ASEnvInfo::LIBRARY_OPTIONS:		return reinterpret_cast<asPWORD>( m_LibOptionsFunc() );
	}

	return 0;
}

#endif //ASMOD_CASSIMPLEENVIRONMENT_H
