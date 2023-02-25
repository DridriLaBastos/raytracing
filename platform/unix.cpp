//
// Created by Adrien COURNAND on 25/02/2023.
//

#include <cstddef>
#include <dlfcn.h>

#include "common.hpp"

module_t dylib_Reload(const char* dylibPath, module_t lastLoad)
{
	dlclose(lastLoad);
	return dlopen(dylibPath,RTLD_LAZY);
}

void dylib_Unload(module_t module)
{
	dlclose(module);
}

void dylib_LoadFunctionWithName(const char* functionName, const module_t moduleID, void** functionPtr)
{
	*functionPtr = dlsym(moduleID,"hotReloadDraw");
}

const char* dylib_GetErrorStr(void)
{
	return dlerror();
}
