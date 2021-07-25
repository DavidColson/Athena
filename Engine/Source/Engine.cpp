// Copyright 2020-2021 David Colson. All rights reserved.

#include "Engine.h"

#include "Core/Log.h"
#include "Core/Memory.h"
#include "AssetDatabase/AssetDatabase.h"
#include "AssetDatabase/Text.h"
#include "AssetDatabase/Shader.h"
#include "AssetDatabase/Mesh.h"
#include "AssetDatabase/Model.h"
#include "AssetDatabase/Image.h"
#include "Core/Vec3.h"
#include "Core/Matrix.h"
#include "Input.h"

#include <SDL.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <EASTL/vector.h>

#include <shaderc/shaderc.h>

// This defines a macro called min somehow? We should avoid it at all costs and include it last
#include <SDL_syswm.h>

void MakeWindow()
{
	{

		Vec3f cameraPos(0.0f, 0.0f, 0.0f);
		Vec3f cameraRot(0.0f, 0.0f, 0.0f);

		Input::CreateInputState();
		eastl::vector<float> vec;
		
		int winWidth = 1600;
		int winHeight = 900;

		An::Log::Debug("Hello");

		SDL_Window* pWindow = SDL_CreateWindow(
			"Flight Game",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			int(winWidth),
			int(winHeight),
			SDL_WINDOW_RESIZABLE
		);

		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(pWindow, &wmInfo);
		HWND hwnd = wmInfo.info.win.window;

		bgfx::Init init;
		init.type = bgfx::RendererType::Direct3D11;
		init.platformData.ndt = NULL;
		init.platformData.nwh = wmInfo.info.win.window;

		bgfx::renderFrame();

		bgfx::init(init);
		const bgfx::ViewId kClearView = 0;
		bgfx::setViewClear(kClearView, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x404040ff, 1.0f, 0);





		An::AssetHandle cubesVertShader = An::AssetHandle("Engine/Shaders/cubes.vs");
		An::AssetHandle cubesFragShader = An::AssetHandle("Engine/Shaders/cubes.fs");
		
		// Create cube input layout
		An::Primitive::InitPrimitiveLayouts();
		An::Primitive cube = An::Primitive::NewCube();

		An::AssetHandle plane("Game/Assets/Spitfire.gltf:mesh_13");
		An::AssetHandle planeColour("Game/Assets/BaseColor.png");


		bgfx::ShaderHandle cacheVertShaderHandle = BGFX_INVALID_HANDLE;
		bgfx::ShaderHandle cacheFragShaderHandle = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle colourSamplerHandle = bgfx::createUniform("s_texColor",  bgfx::UniformType::Sampler);
		bgfx::UniformHandle lightDirectionHandle = bgfx::createUniform("u_lightDir", bgfx::UniformType::Vec4);





		bgfx::setViewRect(kClearView, 0, 0, winWidth, winHeight);
		bgfx::reset(winWidth, winHeight, BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X8);
		bool gameRunning = true;
		float deltaTime = 0.016f;
	    Vec2i relativeMouseStartLocation{ Vec2i(0, 0) };
		bool isCapturingMouse = false;
		while (gameRunning)
		{
			Uint64 frameStart = SDL_GetPerformanceCounter();
			Input::ClearState();

			// Deal with events
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				Input::ProcessEvent(&event);
				switch (event.type)
				{
				case SDL_KEYDOWN:
					{
						if (event.key.keysym.scancode == SDL_SCANCODE_TAB && event.key.keysym.mod & KMOD_LSHIFT)
						{
							isCapturingMouse = !isCapturingMouse;
							if (isCapturingMouse)
							{
								SDL_GetGlobalMouseState(&relativeMouseStartLocation.x, &relativeMouseStartLocation.y);
								SDL_SetRelativeMouseMode(SDL_TRUE);
							}
							else
							{
								SDL_SetRelativeMouseMode(SDL_FALSE);
								SDL_WarpMouseGlobal(relativeMouseStartLocation.x, relativeMouseStartLocation.y);
							}
						}
					}
					break;
				case SDL_WINDOWEVENT:
					switch (event.window.event)
					{
					case SDL_WINDOWEVENT_CLOSE:
						gameRunning = false;
						break;
					default:
						break;
					}
					break;
				case SDL_QUIT:
					gameRunning = false;
					break;
				}
			}

			// Camera control
			{
				const float camSpeed = 5.0f;
			
				Matrixf toCameraSpace = Quatf::MakeFromEuler(cameraRot).ToMatrix(); //??
				
				Vec3f right = toCameraSpace.GetRightVector().GetNormalized();
				//Vec3f right = Vec3f(1.0f, 0.0f, 0.0f);
				if (Input::GetKeyHeld(SDL_SCANCODE_A))
					cameraPos += right * camSpeed * deltaTime;
				if (Input::GetKeyHeld(SDL_SCANCODE_D))
					cameraPos -= right * camSpeed * deltaTime;
					
				Vec3f forward = toCameraSpace.GetForwardVector().GetNormalized();
				if (Input::GetKeyHeld(SDL_SCANCODE_W))
					cameraPos += forward * camSpeed * deltaTime;
				if (Input::GetKeyHeld(SDL_SCANCODE_S))
					cameraPos -= forward * camSpeed * deltaTime;

				Vec3f up = toCameraSpace.GetUpVector().GetNormalized();
				if (Input::GetKeyHeld(SDL_SCANCODE_SPACE))
					cameraPos -= up * camSpeed * deltaTime;
				if (Input::GetKeyHeld(SDL_SCANCODE_LCTRL))
					cameraPos += up * camSpeed * deltaTime;

				if (Input::GetMouseInRelativeMode())
				{
					cameraRot.y -= 0.1f * deltaTime * Input::GetMouseDelta().x;
					cameraRot.x -= 0.1f * deltaTime * Input::GetMouseDelta().y;
				}
			}

			bgfx::touch(kClearView);

			bgfx::ShaderHandle vShaderHandle = An::AssetDB::GetAsset<An::Shader>(cubesVertShader)->m_handle;
			bgfx::ShaderHandle fShaderHandle = An::AssetDB::GetAsset<An::Shader>(cubesFragShader)->m_handle;

			if (vShaderHandle.idx != cacheFragShaderHandle.idx || fShaderHandle.idx != cacheFragShaderHandle.idx)
			{
				if (bgfx::isValid(program))
					bgfx::destroy(program);
				program = bgfx::createProgram(vShaderHandle, fShaderHandle, false);
				cacheFragShaderHandle = fShaderHandle;
				cacheVertShaderHandle = vShaderHandle;
			}


			Quatf rotation = Quatf::MakeFromEuler(cameraRot);
			Matrixf camera = Matrixf::MakeLookAt(rotation.GetForwardVector(), rotation.GetUpVector()) * Matrixf::MakeTranslation(cameraPos);
			Matrixf project = Matrixf::Perspective((float)winWidth, (float)winHeight, 0.1f, 100.0f, 60.0f);

			Vec4f lightDir(0.0f, 2.5f, -1.6f, 1.0f);

			bgfx::setViewTransform(0, &camera, &project);


			uint64_t state = 0
					| BGFX_STATE_WRITE_R
					| BGFX_STATE_WRITE_G
					| BGFX_STATE_WRITE_B
					| BGFX_STATE_WRITE_A
					| BGFX_STATE_WRITE_Z
					| BGFX_STATE_DEPTH_TEST_LESS
					| BGFX_STATE_MSAA;

			Matrixf rotate = Matrixf::MakeRotation(Vec3f(-1.5f, 0.8f, 0.0f));

			// Set model matrix for rendering.
			bgfx::setTransform(&rotate);

			An::Mesh* pPlaneRaw = An::AssetDB::GetAsset<An::Mesh>(plane);
			An::Primitive* pPlaneFuze = &(pPlaneRaw->m_primitives[0]);

			bgfx::setVertexBuffer(0, pPlaneFuze->m_vertexBuffer);
			bgfx::setVertexBuffer(1, pPlaneFuze->m_uv0Buffer);
			bgfx::setVertexBuffer(2, pPlaneFuze->m_normalsBuffer);
			bgfx::setIndexBuffer(pPlaneFuze->m_indexBuffer);

			An::Image* pPlaneImage = An::AssetDB::GetAsset<An::Image>(planeColour);
			bgfx::setTexture(0, colourSamplerHandle,  pPlaneImage->m_gpuHandle);

			bgfx::setUniform(lightDirectionHandle, &lightDir);

			bgfx::setState(state);
			bgfx::submit(0, program);

			bgfx::dbgTextClear();
			bgfx::dbgTextPrintf(10, 10, 0x0f, "Hello world");
			bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS);
			bgfx::frame();

			An::AssetDB::UpdateHotReloading();

			deltaTime = float(SDL_GetPerformanceCounter() - frameStart) / SDL_GetPerformanceFrequency();
		}
		cube.Destroy();
	}
	An::AssetDB::CollectGarbage();
	bgfx::shutdown();
}