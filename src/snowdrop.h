#pragma once
#include <cstdint>
#include "maths.h"

#define SD_NULL nullptr

typedef uint64_t SdSize;

enum SdResult
{
	SD_SUCCESS,
	SD_ERROR,
};

enum SdIndexType
{
	SD_INDEX_TYPE_16_BIT,
	SD_INDEX_TYPE_32_BIT,
};

enum SdBufferUsage
{
	SD_BUFFER_USAGE_NONE,
	SD_BUFFER_USAGE_VERTEX,
	SD_BUFFER_USAGE_INDEX,
};

struct SdBufferCreateInfo
{
	SdSize size;
	SdBufferUsage usage;
	SdIndexType indexType; // only if usage is index
	uint32_t stride;       // only if usage is vertex (bytes)
};

struct SdBuffer_t
{
	SdBufferUsage usage;
	SdIndexType indexType;
	SdSize size;
	uint32_t stride; // (bytes)
	void* data;
};
typedef SdBuffer_t* SdBuffer;

inline extern SdResult sdCreateBuffer(const SdBufferCreateInfo* createInfo, SdBuffer* buffer);
inline extern void     sdDestroyBuffer(SdBuffer buffer);
inline extern void*    sdAccessBuffer(SdBuffer buffer);

struct SdFramebufferCreateInfo
{

};

struct SdFramebuffer_t
{

};
typedef SdFramebuffer_t* SdFramebuffer;

inline extern SdResult sdCreateFramebuffer(const SdFramebufferCreateInfo* createInfo, SdFramebuffer* framebuffer);
inline extern void     sdDestroyFramebuffer(SdFramebuffer framebuffer);

inline extern SdResult sdDraw(SdBuffer vertexBuffer);
inline extern SdResult sdDrawIndexed(SdBuffer VertexBuffer, SdBuffer indexBuffer);

inline extern void sdSetVertexProcessorFunction(vec4 (*vertProc)(const void*));
inline extern void sdSetFragmentProcessorFunction(vec4(*fragProc)(const vec2));
inline extern void sdSetExternalRenderTarget(uint32_t* data, uint32_t width, uint32_t height);