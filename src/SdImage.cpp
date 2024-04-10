#include <snowdrop/snowdrop.h>

uint32_t GetStride(SdImageFormat format)
{
	switch (format)
	{
	case SD_FORMAT_R8G8B8A8_UNORM: return sizeof(uint32_t);
	case SD_FORMAT_R8_UNORM:       return sizeof(uint8_t);
	}
	return 0;
}

SdResult sdCreateImage(const SdImageCreateInfo* createInfo, SdImage* image)
{
	SdImage_t* result = new SdImage_t();

	uint32_t stride = GetStride(createInfo->format);
	SdSize size = createInfo->width * createInfo->height * stride;
	result->data = new uint8_t[size];
	result->width = createInfo->width;
	result->height = createInfo->height;
	result->flags = createInfo->flags;
	result->format = createInfo->format;
	result->stride = stride;

	*image = result;
	return SD_SUCCESS;
}

void sdDestroyImage(SdImage image)
{
	if (!(image->flags & SD_IMAGE_EXTERNAL_BIT))
		delete[] image->data;
	delete image;
}

void* sdAccessImage(SdImage image)
{
	return image->data;
}

SdResult sdImportImage(const SdImageImportInfo* importInfo, SdImage* image)
{
	SdImage_t* result = new SdImage_t();
	
	result->stride = GetStride(importInfo->format);
	result->data = importInfo->data;
	result->width = importInfo->width;
	result->height = importInfo->height;
	result->flags = SD_IMAGE_EXTERNAL_BIT;
	if (importInfo->flipY)
		result->flags |= SD_IMAGE_FLIP_Y_BIT;

	*image = result;
	return SD_SUCCESS;
}

SdResult sdCreateFramebuffer(const SdFramebufferCreateInfo* createInfo, SdFramebuffer* framebuffer)
{
	SdFramebuffer_t* result = new SdFramebuffer_t();

	result->images = new SdImage[createInfo->imageCount];
	result->imageCount = createInfo->imageCount;
	result->flags = createInfo->flags;
	result->cull = createInfo->cullMode;

	*framebuffer = result;
	return SD_SUCCESS;
}

void sdDestroyFramebuffer(SdFramebuffer framebuffer)
{
	for (uint32_t i = 0; i < framebuffer->imageCount; i++)
		if (framebuffer->images[i]) sdDestroyImage(framebuffer->images[i]); // this assumes ownership of the image
	if (framebuffer->depth) sdDestroyImage(framebuffer->depth);
	if (framebuffer->imageCount > 0) delete[] framebuffer->images;
	delete framebuffer;
}

SdResult sdFramebufferBindImage(SdFramebuffer framebuffer, SdImage image, int index)
{
	if (index == SD_DEPTH_INDEX)
		framebuffer->depth = image;
	else
		framebuffer->images[index] = image;
	return SD_SUCCESS;
}

vec4 sdSampleTexture(SdImage image, vec2 uv)
{
	uint32_t absX = image->width * uv.x;
	uint32_t absY = image->height * uv.y;

	int index = (absX * image->height + absY) * image->stride;
	uint8_t* converted = (uint8_t*)image->data + index;
	return vec4(converted[0] / 255.0f, converted[1] / 255.0f, converted[2] / 255.0f, converted[3] / 255.0f);
}