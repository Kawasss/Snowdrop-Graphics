#pragma once
#include <cstdint>
#include "maths.h"

#define SD_NULL nullptr

typedef uint64_t SdSize;
typedef uint32_t SdFlags;

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

enum SdImageFlags : SdFlags
{
	SD_IMAGE_EXTERNAL_BIT = 1 << 0,
	SD_IMAGE_FLIP_Y_BIT = 1 << 1,
};

struct SdImageImportInfo
{
	void* data;
	uint32_t width;
	uint32_t height;
	bool flipY;
};

struct SdImageCreateInfo
{
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	SdFlags flags;
	int type; // unused
};

struct SdImage_t
{
	void* data;
	uint32_t width;
	uint32_t height;
	SdFlags flags;
	int type; // unused
};
typedef SdImage_t* SdImage;

inline extern SdResult sdCreateImage(const SdImageCreateInfo* createInfo, SdImage* image);
inline extern void     sdDestroyImage(SdImage image);
inline extern void*    sdAccessImage(SdImage image);
inline extern SdResult sdImportImage(const SdImageImportInfo* importInfo, SdImage* image);

struct SdFramebufferCreateInfo
{
	uint32_t imageCount;
};

struct SdFramebuffer_t
{
	SdImage* images;
	uint32_t imageCount;
};
typedef SdFramebuffer_t* SdFramebuffer;

inline extern SdResult sdCreateFramebuffer(const SdFramebufferCreateInfo* createInfo, SdFramebuffer* framebuffer);
inline extern void     sdDestroyFramebuffer(SdFramebuffer framebuffer);
inline extern SdResult sdBindImage(SdFramebuffer framebuffer, SdImage image, uint32_t index);

inline extern SdResult sdDraw(SdBuffer vertexBuffer);
inline extern SdResult sdDrawIndexed(SdBuffer vertexBuffer, SdBuffer indexBuffer);
inline extern SdResult sdBindFramebuffer(SdFramebuffer framebuffer);

inline extern void sdSetVertexProcessorFunction(vec4 (*vertProc)(const void*));
inline extern void sdSetFragmentProcessorFunction(vec4(*fragProc)(const vec2));

inline extern vec4 sdTexture(SdImage image, vec2 uv);