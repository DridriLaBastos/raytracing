#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <cstdint>

using module_t = void*;
constexpr module_t INVALID_MODULE_ID = nullptr;

union Pixel
{
	uint32_t value;
	struct
	{
		uint32_t red:8,green:8,blue:8,alpha:8;
	} __attribute__((packed));
} __attribute__((packed));

struct DrawInfo
{
	size_t pixelBufferSize;
	float scaleFactor;
};

module_t dylib_Reload(const char* dylibPath, module_t lastLoad);
void dylib_Unload(module_t lastLoad);
void dylib_LoadFunctionWithName(const char* functionName, const module_t moduleID, void** functionPtr);
const char* dylib_GetErrorStr(void);


#endif