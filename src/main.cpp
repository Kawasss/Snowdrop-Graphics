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

	vec2 vertices[3] = { { -0.5f, -0.5f }, { 0, 0.5f }, { 0.5f, -0.5f } };

	sdSetExternalRenderTarget((uint32_t*)surface->pixels, surface->w, surface->h);
	sdSetVertexProcessorFunction(&VertexProcessor);
	sdSetFragmentProcessorFunction(&FragmentProcessor);

	SdBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.usage = SD_BUFFER_USAGE_VERTEX;
	bufferCreateInfo.stride = sizeof(vec2);
	bufferCreateInfo.size = sizeof(vec2) * 3;

	SdBuffer vertexBuffer = SD_NULL;
	if (sdCreateBuffer(&bufferCreateInfo, &vertexBuffer) != SD_SUCCESS)
		return 3;

	void* bufferData = sdAccessBuffer(vertexBuffer);
	memcpy(bufferData, vertices, sizeof(vec2) * 3);

	while (!close)
	{
		ProcessSDLEvents(window);
		SDL_LockSurface(surface);
		memset(surface->pixels, 0, surface->w * surface->h * sizeof(uint32_t));
		sdDraw(vertexBuffer);
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