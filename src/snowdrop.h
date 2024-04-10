#pragma once
#include <cstdint>
#include "maths.h"

#define SD_NULL nullptr
#define SD_DEPTH_INDEX -1

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

struct SdContextCreateInfo
{
	uint32_t threadCount;
};

inline extern SdResult sdCreateContext(const SdContextCreateInfo* createInfo);

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

inline extern SdResult sdCreateBuffer(const SdBufferCreateInfo* createInfo, SdBuffer* pBuffer);
inline extern void     sdDestroyBuffer(SdBuffer buffer);
inline extern void*    sdAccessBuffer(SdBuffer buffer);

enum SdImageFormat
{
	SD_FORMAT_R8G8B8A8_UNORM,
	SD_FORMAT_R8_UNORM,
};

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
	SdImageFormat format;
};

struct SdImageCreateInfo
{
	uint32_t width;
	uint32_t height;
	SdFlags flags;
	SdImageFormat format; // unused
};

struct SdImage_t
{
	void* data;
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	SdFlags flags;
	SdImageFormat format;
};
typedef SdImage_t* SdImage;

inline extern SdResult sdCreateImage(const SdImageCreateInfo* createInfo, SdImage* pImage);
inline extern void     sdDestroyImage(SdImage image);
inline extern void*    sdAccessImage(SdImage image);
inline extern SdResult sdImportImage(const SdImageImportInfo* importInfo, SdImage* pImage);

enum SdFramebufferFlags : SdFlags
{
	SD_FRAMEBUFFER_DEPTH_BIT = 1 << 0,
};

enum SdCullMode
{
	SD_CULL_NONE,
	SD_CULL_BACK,
	SD_CULL_FRONT,
};

struct SdFramebufferCreateInfo
{
	uint32_t imageCount;
	SdFramebufferFlags flags;
	SdCullMode cullMode;
};

struct SdFramebuffer_t
{
	SdImage* images;
	uint32_t imageCount;
	SdImage depth;
	SdCullMode cull;
	SdFramebufferFlags flags;
};
typedef SdFramebuffer_t* SdFramebuffer;

inline extern SdResult sdCreateFramebuffer(const SdFramebufferCreateInfo* createInfo, SdFramebuffer* pFramebuffer);
inline extern void     sdDestroyFramebuffer(SdFramebuffer framebuffer);
inline extern SdResult sdFramebufferBindImage(SdFramebuffer framebuffer, SdImage image, int index);

enum SdIOType
{
	SD_IO_FLOAT,
	SD_IO_VEC2,
	SD_IO_VEC3,
	SD_IO_VEC4,
};

struct SdIOVariableDescription
{
	SdIOType type;
	uint32_t offset;
};

struct SdShaderGroupCreateInfo
{
	vec4(*vertexProcessor)(const void*, void*);
	vec4(*fragmentProcessor)(const void*);

	uint32_t ioVarSize;
	uint32_t varDescriptionCount;
	const SdIOVariableDescription* varDescriptions;
};

struct SdShaderGroup_t
{
	vec4(*vertProc)(const void*, void*);
	vec4(*fragProc)(const void*);

	uint32_t ioVarSize;
	uint32_t varDescriptionCount;
	SdIOVariableDescription* varDescriptions;
};
typedef SdShaderGroup_t* SdShaderGroup;

inline extern SdResult sdCreateShaderGroup(const SdShaderGroupCreateInfo* createInfo, SdShaderGroup* pShaderGroup);
inline extern void sdBindShaderGroup(SdShaderGroup shaderGroup);
inline extern void sdDestroyShaderGroup(SdShaderGroup shaderGroup);

inline extern void sdClearFramebuffer(SdFramebuffer framebuffer, uint8_t color);
inline extern void sdClearImage(SdImage image, uint8_t color);
inline extern SdResult sdDraw(SdBuffer vertexBuffer);
inline extern SdResult sdDrawIndexed(SdBuffer vertexBuffer, SdBuffer indexBuffer);
inline extern SdResult sdBindFramebuffer(SdFramebuffer framebuffer);

inline extern vec4 sdTexture(SdImage image, vec2 uv);