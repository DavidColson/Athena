// Copyright 2020-2021 David Colson. All rights reserved.

#include "Engine.h"
#include "AssetDatabase/Shader.h"
#include "AssetDatabase/Mesh.h"
#include "AssetDatabase/Model.h"
#include "AssetDatabase/Image.h"
#include "Core/Vec3.h"
#include "Core/Matrix.h"
#include "Input.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <SDL.h>

namespace An
{
	void RecursiveTransformTree(Node* pTreeBase)
	{
		for (Node* pChild : pTreeBase->m_children)
		{
			Matrixf childLocalTrans = Matrixf::MakeTQS(pChild->m_translation, pChild->m_rotation, pChild->m_scale);
			pChild->m_worldTransform = pTreeBase->m_worldTransform * childLocalTrans;

			if (!pChild->m_children.empty())
				RecursiveTransformTree(pChild);
		}
	}

	void TransformNodeHeirarchy(eastl::vector<Node>& nodeList)
	{
		eastl::vector<Node*> rootTransforms;

		for (Node& node : nodeList)
		{
			if (node.m_pParent == nullptr)
			{
				node.m_worldTransform = Matrixf::MakeTQS(node.m_translation, node.m_rotation, node.m_scale);

				if (!node.m_children.empty())
					rootTransforms.push_back(&node);
			}
		}

		for (Node* pRoot : rootTransforms)
		{
			RecursiveTransformTree(pRoot);
		}	
	}
}

int main(int argc, char *argv[])
{
	using namespace An;

	int width = 1600;
	int height = 900;
	InitWindow(1600, 900);

	Vec3f cameraPos(0.0f, 0.0f, 0.0f);
	Vec3f cameraRot(0.0f, 0.0f, 0.0f);

	Input::CreateInputState();
	eastl::vector<float> vec;


	Shader basicVertShader = Shader("Engine/Shaders/cubes.vs");
	Shader basicFragShader = Shader("Engine/Shaders/cubes.fs");

	Scene plane("Game/Assets/Spitfire.gltf");
	Image planeColour = Image("Game/Assets/BaseColor.png");

	Node& master = plane.m_nodes[1];
	master.m_rotation = Quatf::MakeFromEuler(Vec3f(-1.57f, 0.0f, 0.0f));

	TransformNodeHeirarchy(plane.m_nodes);

	bgfx::ShaderHandle cacheVertShaderHandle = BGFX_INVALID_HANDLE;
	bgfx::ShaderHandle cacheFragShaderHandle = BGFX_INVALID_HANDLE;
	bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;

	bgfx::UniformHandle colourSamplerHandle = bgfx::createUniform("s_texColor",  bgfx::UniformType::Sampler);
	bgfx::UniformHandle lightDirectionHandle = bgfx::createUniform("u_lightDir", bgfx::UniformType::Vec4);

	while (!ShouldWindowClose())
	{
		float deltaTime = StartFrame();

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

		// do world space transforms

		

		bgfx::ShaderHandle vShaderHandle = basicVertShader.m_handle;
		bgfx::ShaderHandle fShaderHandle = basicFragShader.m_handle;

		// Reloads the program if a shader reloaded
		// TODO: Make the program an asset type that can be constructed with multiple shader paths, no need for this then
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
		Matrixf project = Matrixf::Perspective((float)width, (float)height, 0.1f, 100.0f, 60.0f);

		Vec4f lightDir(0.0f, -2.5f, -1.6f, 1.0f);

		bgfx::setViewTransform(0, &camera, &project);


		uint64_t state = 0
				| BGFX_STATE_WRITE_R
				| BGFX_STATE_WRITE_G
				| BGFX_STATE_WRITE_B
				| BGFX_STATE_WRITE_A
				| BGFX_STATE_WRITE_Z
				| BGFX_STATE_DEPTH_TEST_LESS
				| BGFX_STATE_MSAA;

		static float x = 0.0f;

		

		bgfx::setUniform(lightDirectionHandle, &lightDir);

		bgfx::setState(state);

		for (Node& node : plane.m_nodes)
		{
			if (node.m_meshId != UINT32_MAX)
			{
				bgfx::setTransform(&node.m_worldTransform);
				
				Primitive* pPrim = &plane.m_meshes[node.m_meshId].m_primitives[0];

				bgfx::setVertexBuffer(0, pPrim->m_vertexBuffer);
				bgfx::setVertexBuffer(1, pPrim->m_uv0Buffer);
				bgfx::setVertexBuffer(2, pPrim->m_normalsBuffer);
				bgfx::setIndexBuffer(pPrim->m_indexBuffer);

				bgfx::setTexture(0, colourSamplerHandle,  planeColour.m_gpuHandle);
				bgfx::submit(0, program);
			}
		}

		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(10, 10, 0x0f, "Hello world");
		
		EndFrame();
	}

	CloseWindow();

	return 0;
}