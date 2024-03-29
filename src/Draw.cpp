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
	vec4 relPixels[3]{};
	relPixels[0] = vertexProc(vert0);
	relPixels[1] = vertexProc(vert1);
	relPixels[2] = vertexProc(vert2);

	vec2 pixelsPos[3]{};
	for (int i = 0; i < 3; i++)
	{
		vec2 screenSpace = (vec2(relPixels[i]) + vec2(1)) * 0.5f;
		pixelsPos[i] = vec2(screenSpace.x * renderTarget->images[0]->width, screenSpace.y * renderTarget->images[0]->height);
	}

	std::future<void> proc[3];
	for (int i = 0; i < 3; i++)
	{
		proc[i] = std::async([=]()
			{
				int j = i == 2 ? 0 : i + 1;
				float distance = GetDistance(pixelsPos[i], pixelsPos[j]);
				float traversed = 0.0f;

				vec2 currentPos = pixelsPos[i] + vec2(1);

				while (traversed < 1.0f)
				{
					vec2 relativePos = currentPos / vec2(renderTarget->images[0]->width, renderTarget->images[0]->height);

					vec4 color = fragmentProc(relativePos);
					uint8_t unorm[4] = { uint8_t(color.r * 255), uint8_t(color.g * 255), uint8_t(color.b * 255), uint8_t(color.a * 255) };
					uint32_t* dest = (uint32_t*)renderTarget->images[0]->data;

					uint64_t y = renderTarget->images[0]->flags & SD_IMAGE_FLIP_Y_BIT ? renderTarget->images[0]->height - currentPos.y : currentPos.y;
					uint64_t index = renderTarget->images[0]->width * y + uint64_t(currentPos.x);

					dest[index] = *(uint32_t*)unorm;

					float dis1 = (float)GetDistance(pixelsPos[i], currentPos);
					currentPos = glm::mix(pixelsPos[i], pixelsPos[j], traversed);

					traversed += distance / (renderTarget->images[0]->width * renderTarget->images[0]->height);
				}
			});
	}
	for (int i = 0; i < 3; i++)
		proc[i].get();
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