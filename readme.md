# Snowdrop Graphics

Snowdrop is a small software graphics library that can be used to render primitives. It solely relies on the CPU for rendering and can be configured to use any amount of threads.

## Documentation

### Context

A context is the space where the functions will work in. To create a context call this function:

```
SdResult sdCreateContext(const SdContextCreateInfo* createInfo);
```

Where ```SdContextCreateInfo``` is defined as follows:

```
struct SdContextCreateInfo
{
	uint32_t threadCount;
};
```

```threadCount``` refers to the maximum amount of threads the rendering process can use. Setting it to 0 will cause undefined behavior.

### Buffers

Buffers are used to create memory that the library can safely manage and use. To create a buffer call this function:

```
SdResult sdCreateBuffer(const SdBufferCreateInfo* createInfo, SdBuffer* pBuffer);
```

Where ```SdBufferCreateInfo``` is defined like this:

```
struct SdBufferCreateInfo
{
	SdSize size;
	SdBufferUsage usage;
	SdIndexType indexType;
	uint32_t stride;
};
```

```size```: the total size of the buffer in bytes.
```usage```: dictates the way that the library uses a buffer.
The usage can be chosen from the ```SdBufferUsage``` enum:

```
enum SdBufferUsage
{
	SD_BUFFER_USAGE_NONE,
	SD_BUFFER_USAGE_VERTEX,
	SD_BUFFER_USAGE_INDEX,
};
```

If ```usage``` is ```SD_BUFFER_USAGE_INDEX```, ```indexType``` must be a valid value from the ```SdIndexType``` enum:

```
enum SdIndexType
{
	SD_INDEX_TYPE_16_BIT,
	SD_INDEX_TYPE_32_BIT,
};
```

The library will interpret the index buffer as a buffer that only contains unsinged 16 or 32 bit integers, depending on the type given.

```stride```: the size of a single vertex in bytes, if ```usage``` is ```SD_BUFFER_USAGE_VERTEX```.

### Images

### Framebuffers

### Shader groups

### Drawing