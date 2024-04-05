#define SDL_MAIN_HANDLED
#include <Windows.h>
#include <iostream>
#include <SDL2/SDL.h>
#include <future>

#include "snowdrop.h"

bool close = false;
SDL_Surface* surface;

struct Vertex
{
	float posx, posy, posz, norx, nory, norz, uvx, uvy;

	union
	{
		vec3 pos;
		vec3 normal;
		vec2 uv;
	};
};

vec2 mouseMotion;
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
		case SDL_MOUSEMOTION:
			mouseMotion += vec2(sdlEvent.motion.xrel, sdlEvent.motion.yrel);
		}
	}
}

float id = 0.0f;
vec2 rotation;
mat4 view, proj, rot;
vec4 VertexProcessor(const void* block, void* output)
{
	vec3* vert = (vec3*)block;

	if (rotation.x > 360) rotation.x = 0;
	if (rotation.y > 360) rotation.y = 0;

	id += 1 / 36.0f;
	return /*proj * view * */rot * vec4(*vert, 1);
}

vec4 FragmentProcessor(const vec2 pos, const void* input)
{
	return vec4(id, 1, 1, 1);
}

SdFramebuffer framebuffer = SD_NULL;
SdBuffer vertexBuffer = SD_NULL;
SdBuffer indexBuffer = SD_NULL;
SdShaderGroup shader = SD_NULL;

void CreateFramebuffer()
{
	SdFramebufferCreateInfo createInfo{};
	createInfo.flags = SD_FRAMEBUFFER_DEPTH_BIT;
	createInfo.imageCount = 1;

	SdImageImportInfo importInfo{};
	importInfo.format = SD_FORMAT_R8G8B8A8_UNORM;
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
	constexpr int size = 36;
	Vertex vertices[size] =
	{
		// back face
		{ -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f }, // bottom-left
		{  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f }, // top-right
		{  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f }, // bottom-right         
		{  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f }, // top-right
		{ -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f }, // bottom-left
		{ -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f }, // top-left
		// front face
		{ -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f }, // bottom-left
		{  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f }, // bottom-right
		{  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f }, // top-right
		{  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f }, // top-right
		{ -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f }, // top-left
		{ -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f }, // bottom-left
		// left face
		{ -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f }, // top-right
		{ -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f }, // top-left
		{ -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f }, // bottom-left
		{ -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f }, // bottom-left
		{ -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f }, // bottom-right
		{ -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f }, // top-right
		// right face
		{  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f }, // top-left
		{  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f }, // bottom-right
		{  1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f }, // top-right         
		{  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f }, // bottom-right
		{  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f }, // top-left
		{  1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f }, // bottom-left     
		// bottom face
		{ -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f }, // top-right
		{  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f }, // top-left
		{  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f }, // bottom-left
		{  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f }, // bottom-left
		{ -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f }, // bottom-right
		{ -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f }, // top-right
		// top face
		{ -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f }, // top-left
		{  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f }, // bottom-right
		{  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f }, // top-right     
		{  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f }, // bottom-right
		{ -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f }, // top-left
		{ -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f }  // bottom-left        
	};

	SdBufferCreateInfo createInfo{};
	createInfo.usage = SD_BUFFER_USAGE_VERTEX;
	createInfo.stride = sizeof(vertices[0]);
	createInfo.size = sizeof(vertices[0]) * size;

	sdCreateBuffer(&createInfo, &vertexBuffer);

	void* bufferData = sdAccessBuffer(vertexBuffer);
	memcpy(bufferData, vertices, sizeof(vertices[0]) * size);
}

void CreateIndexBuffer()
{
	constexpr int size = 36;
	uint16_t indices[size] = { 0, 1, 2, 1, 2, 3 };
	for (int i = 0; i < size; i++)
		indices[i] = i;

	SdBufferCreateInfo createInfo{};
	createInfo.usage = SD_BUFFER_USAGE_INDEX;
	createInfo.indexType = SD_INDEX_TYPE_16_BIT;
	createInfo.stride = sizeof(uint16_t);
	createInfo.size = sizeof(uint16_t) * size;

	sdCreateBuffer(&createInfo, &indexBuffer);

	void* bufferData = sdAccessBuffer(indexBuffer);
	memcpy(bufferData, indices, sizeof(uint16_t) * size);
}

void CreateDepthImage()
{
	SdImageCreateInfo createInfo{};
	createInfo.format = SD_FORMAT_R8_UNORM;
	createInfo.width = 800;
	createInfo.height = 600;
	
	SdImage depth = SD_NULL;
	sdCreateImage(&createInfo, &depth);
	sdFramebufferBindImage(framebuffer, depth, SD_DEPTH_INDEX);
}

void CreateShaderGroup()
{
	SdShaderGroupCreateInfo createInfo{};
	createInfo.fragmentProcessor = &FragmentProcessor;
	createInfo.vertexProcessor = &VertexProcessor;
	createInfo.ioVarSize = 0;

	sdCreateShaderGroup(&createInfo, &shader);

	sdBindShaderGroup(shader);
}

void CreateContext()
{
	SdContextCreateInfo createInfo{};
	createInfo.threadCount = 12;

	sdCreateContext(&createInfo);
}

void CleanUp()
{
	sdDestroyShaderGroup(shader);
	sdDestroyBuffer(vertexBuffer);
	sdDestroyBuffer(indexBuffer);
	sdDestroyFramebuffer(framebuffer);
}

#include <iostream>
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

	CreateContext();

	CreateShaderGroup();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateFramebuffer();
	CreateDepthImage();

	while (!close)
	{
		ProcessSDLEvents(window);
		SDL_LockSurface(surface);
		memset(surface->pixels, 0, surface->w * surface->h * sizeof(uint32_t));
		sdDrawIndexed(vertexBuffer, indexBuffer);
		SDL_UnlockSurface(surface);
		SDL_UpdateWindowSurface(window);

		rotation += mouseMotion;
		rot = /*glm::rotate(glm::mat4(1), glm::radians(rotation.x), glm::vec3(1, 0, 0)) * */glm::rotate(glm::mat4(1), glm::radians(rotation.x), glm::vec3(0, 1, 0));
		view = glm::lookAt(vec3(0), vec3(1, 0, 0), vec3(0, 1, 0));
		proj = glm::perspective(glm::radians(90.0f), (float)surface->w / (float)surface->h, 0.01f, 1000.0f);
		proj[1][1] *= -1;
		mouseMotion = vec2(0);

		frameDelta = std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::high_resolution_clock::now() - timeSinceLastFrame).count();
		timeSinceLastFrame = std::chrono::high_resolution_clock::now();

		std::cout << frameDelta << '\n';
	}

	CleanUp();

	SDL_DestroyWindowSurface(window);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}