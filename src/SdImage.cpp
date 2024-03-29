#include "snowdrop.h"

SdResult sdCreateImage(const SdImageCreateInfo* createInfo, SdImage* image)
{
	SdImage_t* result = new SdImage_t();

	SdSize size = createInfo->width * createInfo->height * createInfo->stride;
	result->data = new uint8_t[size];
	result->width = createInfo->width;
	result->height = createInfo->height;
	result->flags = createInfo->flags;
	result->type = createInfo->type;

	*image = result;
	return SD_SUCCESS;
}

void sdDestroyImage(SdImage image)
{
	if (!(image->flags & SD_IMAGE_EXTERNAL_BIT))
		delete image->data;
	delete image;
}

void* sdAccessImage(SdImage image)
{
	return image->data;
}

SdResult sdImportImage(const SdImageImportInfo* importInfo, SdImage* image)
{
	SdImage_t* result = new SdImage_t();
	
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

	*framebuffer = result;
	return SD_SUCCESS;
}

void sdDestroyFramebuffer(SdFramebuffer framebuffer)
{
	for (uint32_t i = 0; i < framebuffer->imageCount; i++)
	{
		if (framebuffer->images[i]) delete framebuffer->images[i]; // this assumes ownership of the image
	}
	delete framebuffer;
}

SdResult sdBindImage(SdFramebuffer framebuffer, SdImage image, uint32_t index)
{
	framebuffer->images[index] = image;
	return SD_SUCCESS;
}

vec4 sdTexture(SdImage image, vec2 uv)
{
	uint32_t absX = image->width * uv.x;
	uint32_t absY = image->height * uv.y;

	uint32_t* data = (uint32_t*)image->data;
	uint32_t raw = data[absY * image->width + absX];
	uint8_t* converted = (uint8_t*)&raw;

	return vec4(converted[0] / 255.0f, converted[1] / 255.0f, converted[2] / 255.0f, converted[3] / 255.0f);
}