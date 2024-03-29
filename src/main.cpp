#define SDL_MAIN_HANDLED
#include <Windows.h>
#include <iostream>
#include <SDL2/SDL.h>
#include <future>

#include "snowdrop.h"

bool close = false;
SDL_Surface* surface;

void ProcessSDLEvents(SDL_Window* window)
{
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent))
	{
		switch (sdlEvent.type)
		{
		case SDL_QUIT:
			close = true;
			break;
		case SDL_WINDOWEVENT:
			close = sdlEvent.window.type == SDL_WINDOWEVENT_CLOSE;
			break;
		}
	}
}

float rotation = 0.0f;
vec4 VertexProcessor(const void* block)
{
	vec2* vert = (vec2*)block;

	if (rotation > 360) rotation = 0;
	mat4 rotMatrix = glm::rotate(glm::mat4(1), glm::radians(rotation), glm::vec3(0, 0, 1));

	return rotMatrix * vec4(*vert, 0, 1);
}

vec4 FragmentProcessor(const vec2 pos)
{
	return vec4(1, 1, 1, 1);
}

SdFramebuffer framebuffer = SD_NULL;
SdBuffer vertexBuffer = SD_NULL;
SdBuffer indexBuffer = SD_NULL;

void CreateFramebuffer()
{
	SdFramebufferCreateInfo createInfo{};
	createInfo.imageCount = 1;

	SdImageImportInfo importInfo{};
	importInfo.data = surface->pixels;
	importInfo.width = surface->w;
	importInfo.height = surface->h;
	importInfo.flipY = true;

	sdCreateFramebuffer(&createInfo, &framebuffer);
	sdImportImage(&importInfo, &framebuffer->images[0]);

	sdBindFramebuffer(framebuffer);
}

void CreateVertexBuffer()
{
	vec2 vertices[4] = { { -0.5f, -0.5f }, { -0.5f, 0.5f }, { 0.5f, -0.5f }, { 0.5f, 0.5f } };

	SdBufferCreateInfo createInfo{};
	createInfo.usage = SD_BUFFER_USAGE_VERTEX;
	createInfo.stride = sizeof(vec2);
	createInfo.size = sizeof(vec2) * 4;

	sdCreateBuffer(&createInfo, &vertexBuffer);

	void* bufferData = sdAccessBuffer(vertexBuffer);
	memcpy(bufferData, vertices, sizeof(vec2) * 4);
}

void CreateIndexBuffer()
{
	uint16_t indices[6] = { 0, 1, 2, 1, 2, 3 };

	SdBufferCreateInfo createInfo{};
	createInfo.usage = SD_BUFFER_USAGE_INDEX;
	createInfo.indexType = SD_INDEX_TYPE_16_BIT;
	createInfo.stride = sizeof(uint16_t);
	createInfo.size = sizeof(uint16_t) * 6;

	sdCreateBuffer(&createInfo, &indexBuffer);

	void* bufferData = sdAccessBuffer(indexBuffer);
	memcpy(bufferData, indices, sizeof(uint16_t) * 6);
}

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return 1;

	SDL_Window* window = SDL_CreateWindow("sdl window!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
	if (!window)
		return 2;

	surface = SDL_GetWindowSurface(window);

	float frameDelta = 0;
	auto timeSinceLastFrame = std::chrono::high_resolution_clock::now();

	sdSetVertexProcessorFunction(&VertexProcessor);
	sdSetFragmentProcessorFunction(&FragmentProcessor);

	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateFramebuffer();

	while (!close)
	{
		ProcessSDLEvents(window);
		SDL_LockSurface(surface);
		memset(surface->pixels, 0, surface->w * surface->h * sizeof(uint32_t));
		sdDrawIndexed(vertexBuffer, indexBuffer);
		SDL_UnlockSurface(surface);
		SDL_UpdateWindowSurface(window);

		rotation += frameDelta * 0.1f;

		frameDelta = std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::high_resolution_clock::now() - timeSinceLastFrame).count();
		timeSinceLastFrame = std::chrono::high_resolution_clock::now();
	}

	sdDestroyBuffer(vertexBuffer);

	SDL_DestroyWindowSurface(window);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}