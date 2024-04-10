#include "snowdrop.h"

SdFramebuffer renderTarget = SD_NULL;
SdShaderGroup currentShader = SD_NULL;

uint32_t threadCount = 1;

SdResult sdCreateContext(const SdContextCreateInfo* createInfo)
{
	threadCount = createInfo->threadCount;
	return SD_SUCCESS;
}

SdResult sdCreateShaderGroup(const SdShaderGroupCreateInfo* createInfo, SdShaderGroup* shaderGroup)
{
	SdShaderGroup_t* ptr = new SdShaderGroup_t();

	ptr->fragProc = createInfo->fragmentProcessor;
	ptr->vertProc = createInfo->vertexProcessor;
	ptr->ioVarSize = createInfo->ioVarSize;
	ptr->varDescriptionCount = createInfo->varDescriptionCount;
	if (createInfo->varDescriptionCount > 0 && createInfo->ioVarSize > 0)
	{
		ptr->varDescriptions = new SdIOVariableDescription[createInfo->varDescriptionCount];
		memcpy(ptr->varDescriptions, createInfo->varDescriptions, sizeof(SdIOVariableDescription) * createInfo->varDescriptionCount);
	}
	
	*shaderGroup = ptr;
	return SD_SUCCESS;
}

void sdBindShaderGroup(SdShaderGroup shaderGroup)
{
	currentShader = shaderGroup;
}

void sdDestroyShaderGroup(SdShaderGroup shaderGroup)
{
	delete[] shaderGroup->varDescriptions;
	delete shaderGroup;
}

SdResult sdBindFramebuffer(SdFramebuffer framebuffer)
{
	renderTarget = framebuffer;
	return SD_SUCCESS;
}

void InterpolateData(void* data0, void* data1, void* data2, void* result, SdIOType type, float s, float t, float p)
{
	switch (type)
	{
	case SD_IO_FLOAT:
	{
		*(float*)result = *(float*)data0 * s + *(float*)data1 * t + *(float*)data2 * p;
		break;
	}
	case SD_IO_VEC2:
	{
		*(vec2*)result = *(vec2*)data0 * s + *(vec2*)data1 * t + *(vec2*)data2 * p;
		break;
	}
	case SD_IO_VEC3:
	{
		*(vec3*)result = *(vec3*)data0 * s + *(vec3*)data1 * t + *(vec3*)data2 * p;
		break;
	}
	case SD_IO_VEC4:
	{
		*(vec4*)result = *(vec4*)data0 * s + *(vec4*)data1 * t + *(vec4*)data2 * p;
		break;
	}
	}
}

float Min(float val, float comp)
{
	return val > comp ? comp : val;
}

float Max(float val, float comp)
{
	return val > comp ? val : comp;
}

float GetDistance(vec2 v1, vec2 v2)
{
	vec2 distance = glm::abs(v2 - v1);
	double ret = sqrt(pow(distance.x, 2) + pow(distance.y, 2));
	return (float)ret;
}

float GetDistance(vec4 v1, vec4 v2)
{
	vec2 distance = glm::abs(v2 - v1);
	double ret = sqrt(pow(distance.x, 2) + pow(distance.y, 2));
	return (float)ret;
}

void Rasterize(void* vert0, void* vert1, void* vert2)
{
	uint32_t width = renderTarget->images[0]->width;
	uint32_t height = renderTarget->images[0]->height;

	uint8_t* ioData[4]{}; // 3 for vertex calls, 4 is for the result
	if (currentShader->ioVarSize > 0)
		for (int i = 0; i < 4; i++)
			ioData[i] = new uint8_t[currentShader->ioVarSize]; // maybe optimize by making one request for memory instead of 4?

	vec4 rel[3]{};
	rel[0] = currentShader->vertProc(vert0, ioData[0]);
	rel[1] = currentShader->vertProc(vert1, ioData[1]);
	rel[2] = currentShader->vertProc(vert2, ioData[2]);

	vec4 abs[3]{}; // the coordinates in pixels, "absolute" coordinates
	vec2 min = vec2(999999), max = vec2(0);
	for (int i = 0; i < 3; i++)
	{
		vec4 screenSpace = (vec4(rel[i]) + vec4(1)) * 0.5f;
		abs[i] = vec4(screenSpace.x * width, screenSpace.y * height, screenSpace.z, screenSpace.w);
		
		max.x = abs[i].x > max.x ? abs[i].x : max.x;
		max.y = abs[i].y > max.y ? abs[i].y : max.y;

		min.x = abs[i].x < min.x ? abs[i].x : min.x;
		min.y = abs[i].y < min.y ? abs[i].y : min.y;
	}
	min.x = ceil(Min(min.x, width));
	min.y = ceil(Min(min.y, height));
	max.x = ceil(Min(max.x, width));
	max.y = ceil(Min(max.y, height));

	float area = 0.5f * (abs[0].x * (abs[1].y - abs[2].y) + abs[1].x * (abs[2].y - abs[0].y) + abs[2].x * (abs[0].y - abs[1].y));//(abs[1].y * abs[2].x + abs[0].y * (-abs[1].x + abs[2].x) + abs[0].x * (abs[1].y - abs[2].y) + abs[1].x * abs[2].y);
	if (area <= 0)
	{
		for (int i = 0; i < 4; i++)
			delete[] ioData[i];
		return;
	}

	float denom = (abs[1].y - abs[2].y) * (abs[0].x - abs[2].x) + (abs[2].x - abs[1].x) * (abs[0].y - abs[2].y) + 0.00001f; // + 0.00...f to prevent dividing by 0
	bool inv = renderTarget->images[0]->flags & SD_IMAGE_FLIP_Y_BIT;
	for (float y = min.y; y < max.y; y++)
	{
		for (float x = min.x; x <= max.x; x++)
		{
			vec2 p = vec2(x, y);

			
			float s = ((abs[1].y - abs[2].y) * (p.x - abs[2].x) + (abs[2].x - abs[1].x) * (p.y - abs[2].y)) / denom;
			float t = ((abs[2].y - abs[0].y) * (p.x - abs[2].x) + (abs[0].x - abs[2].x) * (p.y - abs[2].y)) / denom;
			float z = 1 - s - t;
			
			if ((s <= 0) || (t <= 0) || (s + t > 1))
				continue;
	
			vec2 relativePos = p / vec2(width, height);

			for (int i = 0; i < currentShader->varDescriptionCount; i++)
			{
				uint32_t offset = currentShader->varDescriptions[i].offset;
				InterpolateData((char*)ioData[0] + offset, (char*)ioData[1] + offset, (char*)ioData[2] + offset, (char*)ioData[3] + offset, currentShader->varDescriptions[i].type, s, t, z);
			}

			vec4 color = currentShader->fragProc(relativePos, ioData[3]);
			uint8_t unorm[4] = { uint8_t(color.r * 255), uint8_t(color.g * 255), uint8_t(color.b * 255), uint8_t(color.a * 255) };
			uint32_t* dest = (uint32_t*)renderTarget->images[0]->data;

			bool write = true;
			uint32_t index = width * (inv ? height - y : y) + x;
			if (renderTarget->flags & SD_FRAMEBUFFER_DEPTH_BIT)
			{
				uint8_t* depthData = (uint8_t*)renderTarget->depth->data;
				float depth = rel[0].z * s + rel[1].z * t + rel[2].z * z;
				uint8_t convert = uint8_t(depth * 255);
				write = depthData[index] >= convert;
				if (write)
					depthData[index] = convert;
				depth++;
			}
			if (write) dest[index] = *(uint32_t*)unorm;	
		}
	}
	for (int i = 0; i < 4; i++)
		delete[] ioData[i];
}

SdResult sdDraw(SdBuffer buffer)
{
	
	for (int i = 0; i < buffer->size; i += buffer->stride * 3)
	{
		Rasterize((char*)buffer->data + i, (char*)buffer->data + i + buffer->stride, (char*)buffer->data + i + buffer->stride * 2);
	}
	return SD_SUCCESS;
}

#include <future>
SdResult sdDrawIndexed(SdBuffer vertexBuffer, SdBuffer indexBuffer)
{
	memset(renderTarget->depth->data, 255, renderTarget->depth->width * renderTarget->depth->height);
	SdSize bufferSize = indexBuffer->size / (indexBuffer->indexType == SD_INDEX_TYPE_16_BIT ? sizeof(uint16_t) : sizeof(uint32_t));
	SdSize parallelSize = bufferSize / threadCount;
	std::future<void>* proc = new std::future<void>[threadCount];
	for (int x = 0; x < threadCount; x++)
	{
		proc[x] = std::async([=]() {
			for (int i = parallelSize * x; i < parallelSize * x + parallelSize; i += 3)
			{
				void* vert0 = nullptr, * vert1 = nullptr, * vert2 = nullptr;
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
			});
	}
	for (int i = 0; i < threadCount; i++)
		proc[i].get();
	return SD_SUCCESS;
}