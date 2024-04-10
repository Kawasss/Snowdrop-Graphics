#define SDL_MAIN_HANDLED
#include <Windows.h>
#include <iostream>
#include <SDL2/SDL.h>
#include <future>

#include <snowdrop/snowdrop.h>

bool close = false;
SDL_Surface* surface;

union Vertex
{
	Vertex(float v0, float v1, float v2, float v3, float v4, float v5, float v6, float v7) : pos(v0, v1, v2), normal(v3, v4, v5), uv(v6, v7) {}

	struct
	{
		vec3 pos;
		vec3 normal;
		vec2 uv;
	};
	float data[8];
};

struct ShaderData
{
	vec3 normal;
	vec2 uv;
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
	Vertex* vert = (Vertex*)block;
	ShaderData* out = (ShaderData*)output;
	out->normal = vert->normal;
	out->uv = vert->uv;

	if (rotation.x > 360) rotation.x = 0;
	if (rotation.y > 360) rotation.y = 0;

	id += 1 / 36.0f;
	return proj * view * rot * vec4(vert->pos, 1);
}

vec4 FragmentProcessor(const void* input)
{
	ShaderData* in = (ShaderData*)input;
	return vec4(in->uv, 0, 1);
}

SdFramebuffer framebuffer = SD_NULL;
SdBuffer vertexBuffer = SD_NULL;
SdBuffer indexBuffer = SD_NULL;
SdShaderGroup shader = SD_NULL;

void CreateFramebuffer()
{
	SdFramebufferCreateInfo createInfo{};
	createInfo.flags = SD_FRAMEBUFFER_DEPTH_BIT;
	createInfo.cullMode = SD_CULL_BACK;
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
		{ -.5f, -.5f, -.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f }, // bottom-left
		{  .5f,  .5f, -.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f }, // top-right
		{  .5f, -.5f, -.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f }, // bottom-right         
		{  .5f,  .5f, -.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f }, // top-right
		{ -.5f, -.5f, -.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f }, // bottom-left
		{ -.5f,  .5f, -.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f }, // top-left
		// front face
		{ -.5f, -.5f,  .5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f }, // bottom-left
		{  .5f, -.5f,  .5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f }, // bottom-right
		{  .5f,  .5f,  .5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f }, // top-right
		{  .5f,  .5f,  .5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f }, // top-right
		{ -.5f,  .5f,  .5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f }, // top-left
		{ -.5f, -.5f,  .5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f }, // bottom-left
		// left face
		{ -.5f,  .5f,  .5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f }, // top-right
		{ -.5f,  .5f, -.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f }, // top-left
		{ -.5f, -.5f, -.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f }, // bottom-left
		{ -.5f, -.5f, -.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f }, // bottom-left
		{ -.5f, -.5f,  .5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f }, // bottom-right
		{ -.5f,  .5f,  .5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f }, // top-right
		// right face
		{  .5f,  .5f,  .5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f }, // top-left
		{  .5f, -.5f, -.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f }, // bottom-right
		{  .5f,  .5f, -.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f }, // top-right         
		{  .5f, -.5f, -.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f }, // bottom-right
		{  .5f,  .5f,  .5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f }, // top-left
		{  .5f, -.5f,  .5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f }, // bottom-left     
		// bottom face
		{ -.5f, -.5f, -.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f }, // top-right
		{  .5f, -.5f, -.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f }, // top-left
		{  .5f, -.5f,  .5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f }, // bottom-left
		{  .5f, -.5f,  .5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f }, // bottom-left
		{ -.5f, -.5f,  .5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f }, // bottom-right
		{ -.5f, -.5f, -.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f }, // top-right
		// top face
		{ -.5f,  .5f, -.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f }, // top-left
		{  .5f,  .5f,  .5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f }, // bottom-right
		{  .5f,  .5f, -.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f }, // top-right     
		{  .5f,  .5f,  .5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f }, // bottom-right
		{ -.5f,  .5f, -.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f }, // top-left
		{ -.5f,  .5f,  .5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f }  // bottom-left        
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
	uint16_t indices[size] = { };
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
	createInfo.height = 800; // weird bug here
	
	SdImage depth = SD_NULL;
	sdCreateImage(&createInfo, &depth);
	sdFramebufferBindImage(framebuffer, depth, SD_DEPTH_INDEX);
}

void CreateShaderGroup()
{
	SdIOVariableDescription varDesc[2]{};
	varDesc[0].type = SD_IO_VEC3;
	varDesc[0].offset = offsetof(ShaderData, ShaderData::normal);
	varDesc[1].type = SD_IO_VEC2;
	varDesc[1].offset = offsetof(ShaderData, ShaderData::uv);

	SdShaderGroupCreateInfo createInfo{};
	createInfo.fragmentProcessor = &FragmentProcessor;
	createInfo.vertexProcessor = &VertexProcessor;
	createInfo.ioVarSize = sizeof(ShaderData);
	createInfo.varDescriptionCount = 2;
	createInfo.varDescriptions = varDesc;

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
		sdClearFramebuffer(framebuffer, 0);
		sdDrawIndexed(vertexBuffer, indexBuffer);
		SDL_UnlockSurface(surface);
		SDL_UpdateWindowSurface(window);

		rotation += mouseMotion;
		rot = /*glm::rotate(glm::mat4(1), glm::radians(rotation.x), glm::vec3(1, 0, 0)) * */glm::rotate(glm::mat4(1), glm::radians(rotation.x), glm::vec3(0, 1, 0));
		view = glm::lookAt(vec3(-4, 1, 0), vec3(0, 0, 0), vec3(0, 1, 0));
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