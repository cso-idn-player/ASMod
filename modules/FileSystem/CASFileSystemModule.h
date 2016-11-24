#ifndef FILESYSTEM_CASFILESYSTEMMODULE_H
#define FILESYSTEM_CASFILESYSTEMMODULE_H

#include "ASMod/Module/CASModBaseModule.h"

class CASFileSystemModule : public CASModBaseModule
{
public:
	typedef CASFileSystemModule ThisClass;
	typedef CASModBaseModule BaseClass;

public:
	CASFileSystemModule() = default;
	~CASFileSystemModule() = default;

	const char* GetName() const override final;

	bool Initialize( const CreateInterfaceFn* pFactories, const size_t uiNumFactories ) override;

	bool Shutdown() override;

private:
	CASFileSystemModule( const CASFileSystemModule& ) = delete;
	CASFileSystemModule& operator=( const CASFileSystemModule& ) = delete;
};

#endif //FILESYSTEM_CASFILESYSTEMMODULE_H
