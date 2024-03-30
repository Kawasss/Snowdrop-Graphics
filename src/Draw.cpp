#include "snowdrop.h"
#include <future>

SdFramebuffer renderTarget = SD_NULL;

vec4(*vertexProc) (const void*) = nullptr;
vec4(*fragmentProc)(const vec2) = nullptr;

void sdSetVertexProcessorFunction(vec4(*vertProc)(const void*))
{
	vertexProc = vertProc;
}

void sdSetFragmentProcessorFunction(vec4(*fragProc)(const vec2))
{
	fragmentProc = fragProc;
}

SdResult sdBindFramebuffer(SdFramebuffer framebuffer)
{
	renderTarget = framebuffer;
	return SD_SUCCESS;
}

float GetDistance(vec2 v1, vec2 v2)
{
	vec2 distance = glm::abs(v2 - v1);
	double ret = sqrt(pow(distance.x, 2) + pow(distance.y, 2));
	return (float)ret;
}

void Rasterize(void* vert0, void* vert1, void* vert2)
{
	uint32_t width = renderTarget->images[0]->width;
	uint32_t height = renderTarget->images[0]->height;

	vec4 relPixels[3]{};
	relPixels[0] = vertexProc(vert0);
	relPixels[1] = vertexProc(vert1);
	relPixels[2] = vertexProc(vert2);

	vec4 pixelsPos[3]{};
	vec2 min = vec2(-1), max = vec2(0);
	for (int i = 0; i < 3; i++)
	{
		vec4 screenSpace = (vec4(relPixels[i]) + vec4(1)) * 0.5f;
		pixelsPos[i] = vec4(screenSpace.x * width, screenSpace.y * height, screenSpace.z, screenSpace.w);
		
		max.x = pixelsPos[i].x > max.x ? pixelsPos[i].x : max.x;
		max.y = pixelsPos[i].y > max.y ? pixelsPos[i].y : max.y;

		if (min == vec2(-1)) min = max;

		min.x = pixelsPos[i].x < min.x ? pixelsPos[i].x : min.x;
		min.y = pixelsPos[i].y < min.y ? pixelsPos[i].y : min.y;
	}
	float area = 0.5f * (pixelsPos[1].y * pixelsPos[2].x + pixelsPos[0].y * (-pixelsPos[1].x + pixelsPos[2].x) + pixelsPos[0].x * (pixelsPos[1].y - pixelsPos[2].y) + pixelsPos[1].x * pixelsPos[2].y);
	if (area <= 0) return; 

	bool inv = renderTarget->images[0]->flags & SD_IMAGE_FLIP_Y_BIT;
	for (uint32_t y = min.y; y < max.y; y++)
	{
		for (uint32_t x = min.x; x <= max.x; x++)
		{
			uint64_t index = width * (inv ? height - y : y) + x;
			if (index > width * height) continue;

			vec4 p = vec4(x, y, 0, 1);

			float s = 1 / (2 * area) * (pixelsPos[0].y * pixelsPos[2].x - pixelsPos[0].x * pixelsPos[2].y + (pixelsPos[2].y - pixelsPos[0].y) * p.x + (pixelsPos[0].x - pixelsPos[2].x) * p.y);
			float t = 1 / (2 * area) * (pixelsPos[0].x * pixelsPos[1].y - pixelsPos[0].y * pixelsPos[1].x + (pixelsPos[0].y - pixelsPos[1].y) * p.x + (pixelsPos[1].x - pixelsPos[0].x) * p.y);

			if ((s < 0) || (t < 0) || (s + t > 1))
				continue;

			vec2 relativePos = p / vec4(width, height, 1, 1);

			vec4 color = fragmentProc(relativePos);
			uint8_t unorm[4] = { uint8_t(color.r * 255), uint8_t(color.g * 255), uint8_t(color.b * 255), uint8_t(color.a * 255) };
			uint32_t* dest = (uint32_t*)renderTarget->images[0]->data;

			bool write = true;
			if (renderTarget->flags & SD_FRAMEBUFFER_DEPTH_BIT)
			{
				uint8_t* depthData = (uint8_t*)renderTarget->depth->data;
				write = depthData[index] > p.z;
			}
			if (write) dest[index] = *(uint32_t*)unorm;	
		}
	}
}

SdResult sdDraw(SdBuffer buffer)
{
	
	for (int i = 0; i < buffer->size; i += buffer->stride * 3)
	{
		Rasterize((char*)buffer->data + i, (char*)buffer->data + i + buffer->stride, (char*)buffer->data + i + buffer->stride * 2);
	}
	return SD_SUCCESS;
}

SdResult sdDrawIndexed(SdBuffer vertexBuffer, SdBuffer indexBuffer)
{
	SdSize bufferSize = indexBuffer->size / (indexBuffer->indexType == SD_INDEX_TYPE_16_BIT ? sizeof(uint16_t) : sizeof(uint32_t));
	for (int i = 0; i < bufferSize; i += 3)
	{
		void* vert0 = nullptr, *vert1 = nullptr, *vert2 = nullptr;
		switch (indexBuffer->indexType)
		{
		case SD_INDEX_TYPE_16_BIT: // a lot of the same stuff here, maybe change??
		{
			uint16_t* data = (uint16_t*)indexBuffer->data;
			uint16_t index0 = *(data + i);
			uint16_t index1 = *(data + i + 1);
			uint16_t index2 = *(data + i + 2);

			vert0 = (char*)vertexBuffer->data + index0 * vertexBuffer->stride;
			vert1 = (char*)vertexBuffer->data + index1 * vertexBuffer->stride;
			vert2 = (char*)vertexBuffer->data + index2 * vertexBuffer->stride;
			break;
		}
		case SD_INDEX_TYPE_32_BIT:
		{
			uint32_t* data = (uint32_t*)indexBuffer->data;
			uint32_t index3 = *(data + i);
			uint32_t index4 = *(data + i + 1);
			uint32_t index5 = *(data + i + 2);

			vert0 = (char*)vertexBuffer->data + index3 * vertexBuffer->stride;
			vert1 = (char*)vertexBuffer->data + index4 * vertexBuffer->stride;
			vert2 = (char*)vertexBuffer->data + index5 * vertexBuffer->stride;
			break;
		}
		}
		Rasterize(vert0, vert1, vert2);
	}

	return SD_SUCCESS;
}