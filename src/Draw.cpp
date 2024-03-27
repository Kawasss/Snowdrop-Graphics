#include "snowdrop.h"
#include <future>

struct RenderTarget
{
	uint32_t* data;
	uint32_t width;
	uint32_t height;
};
RenderTarget extRenderTarget;

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

void sdSetExternalRenderTarget(uint32_t* data, uint32_t width, uint32_t height)
{
	extRenderTarget = { data, width, height };
}

float GetDistance(vec2 v1, vec2 v2)
{
	vec2 distance = glm::abs(v2 - v1);
	double ret = sqrt(pow(distance.x, 2) + pow(distance.y, 2));
	return (float)ret;
}

SdResult sdDraw(SdBuffer buffer)
{
	vec4 relPixels[3]{};
	for (int i = 0; i < buffer->size; i += buffer->stride * 3)
	{
		relPixels[0] = vertexProc((char*)buffer->data + i);
		relPixels[1] = vertexProc((char*)buffer->data + i + buffer->stride);
		relPixels[2] = vertexProc((char*)buffer->data + i + buffer->stride * 2);

		vec2 pixelsPos[3]{};
		for (int i = 0; i < 3; i++)
		{
			vec2 screenSpace = (vec2(relPixels[i]) + vec2(1)) * 0.5f;
			pixelsPos[i] = vec2(screenSpace.x * extRenderTarget.width, screenSpace.y * extRenderTarget.height);
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
				
				while (GetDistance(pixelsPos[j], pixelsPos[i]) > GetDistance(currentPos, pixelsPos[i]))
				{
					vec4 color = fragmentProc(currentPos / vec2(extRenderTarget.width, extRenderTarget.height));
					uint8_t unorm[4] = { uint8_t(color.r * 255), uint8_t(color.g * 255), uint8_t(color.b * 255), uint8_t(color.a * 255) };
					extRenderTarget.data[extRenderTarget.width * uint32_t(extRenderTarget.height - currentPos.y) + uint32_t(currentPos.x)] = *(uint32_t*)unorm; // [0, 0] is at the bottom left of the screen, but SDL's is at the top left, hence the "extRenderTarget.height - currentPos.y"

					float dis1 = (float)GetDistance(pixelsPos[i], currentPos);
					currentPos = glm::mix(pixelsPos[i], pixelsPos[j], traversed);

					traversed += distance / (extRenderTarget.width * extRenderTarget.height);
				}
			});
		}
		for (int i = 0; i < 3; i++)
			proc[i].get();
	}
	return SD_SUCCESS;
}