// Copyright 2020-2021 David Colson. All rights reserved.

#include "Engine.h"
#include "AssetDatabase/Shader.h"
#include "AssetDatabase/Mesh.h"
#include "AssetDatabase/Model.h"
#include "AssetDatabase/Image.h"
#include "Core/Vec3.h"
#include "Core/Matrix.h"
#include "Input.h"
#include "Core/Log.h"

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

	struct RendererState
	{
		bgfx::UniformHandle m_baseColorUniform;
		bgfx::UniformHandle m_baseColorTextureSampler;
		bgfx::UniformHandle m_lightDirectionUniform;
		bgfx::ProgramHandle m_texturedProgram;
		bgfx::ProgramHandle m_untexturedProgram;
	};

	void RenderScene(Scene& scene, RendererState& renderer)
	{
		for (Node& node : scene.m_nodes)
		{
			if (node.m_meshId != UINT32_MAX)
			{
				for (Primitive& prim : scene.m_meshes[node.m_meshId].m_primitives)
				{
					bgfx::setTransform(&node.m_worldTransform);
					
					bgfx::setVertexBuffer(0, prim.m_vertexBuffer);
					bgfx::setVertexBuffer(1, prim.m_uv0Buffer);
					bgfx::setVertexBuffer(2, prim.m_normalsBuffer);
					bgfx::setIndexBuffer(prim.m_indexBuffer);

					if (prim.m_baseColorTexture != UINT32_MAX) // Textured
					{
						uint64_t state = 0
						| BGFX_STATE_WRITE_RGB
						| BGFX_STATE_WRITE_Z
						| BGFX_STATE_DEPTH_TEST_LESS
						| BGFX_STATE_MSAA;
						bgfx::setState(state);


						Image& image = scene.m_images[prim.m_baseColorTexture];
						bgfx::setTexture(0, renderer.m_baseColorTextureSampler,  image.m_gpuHandle);
						bgfx::submit(0, renderer.m_texturedProgram);
					}
					else if (prim.m_baseColor.w < 1.0f) // Transparent material
					{
						uint64_t state = 0
						| BGFX_STATE_WRITE_RGB
						| BGFX_STATE_WRITE_A
						| BGFX_STATE_DEPTH_TEST_LESS
						| BGFX_STATE_MSAA
						| BGFX_STATE_BLEND_ALPHA;
						bgfx::setState(state);

						bgfx::setUniform(renderer.m_baseColorUniform, &prim.m_baseColor);
						bgfx::submit(0, renderer.m_untexturedProgram);
					}
					else	// No transparency, no texture
					{
						uint64_t state = 0
						| BGFX_STATE_WRITE_RGB
						| BGFX_STATE_WRITE_Z
						| BGFX_STATE_DEPTH_TEST_LESS
						| BGFX_STATE_MSAA;
						bgfx::setState(state);

						bgfx::setUniform(renderer.m_baseColorUniform, &prim.m_baseColor);
						bgfx::submit(0, renderer.m_untexturedProgram);
					}
				}
			}
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

	RendererState rState;

	Shader basicVertShader = Shader("Engine/Shaders/default.vs");
	Shader texturedLitShader = Shader("Engine/Shaders/texturedLit.fs");
	Shader untexturedLitShader = Shader("Engine/Shaders/untexturedLit.fs");

	rState.m_texturedProgram = bgfx::createProgram(basicVertShader.m_handle, texturedLitShader.m_handle, false);
	rState.m_untexturedProgram = bgfx::createProgram(basicVertShader.m_handle, untexturedLitShader.m_handle, false);

	Scene plane("Game/Assets/Spitfire.gltf");
	Scene terrain("Game/Assets/FirstTerrain.gltf");

	TransformNodeHeirarchy(terrain.m_nodes);
	TransformNodeHeirarchy(plane.m_nodes);

	bgfx::ShaderHandle cacheVertShaderHandle = BGFX_INVALID_HANDLE;
	bgfx::ShaderHandle cacheFragShaderHandle = BGFX_INVALID_HANDLE;
	bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;

	rState.m_baseColorTextureSampler = bgfx::createUniform("s_texColor",  bgfx::UniformType::Sampler);
	rState.m_lightDirectionUniform = bgfx::createUniform("u_lightDir", bgfx::UniformType::Vec4);
	rState.m_baseColorUniform = bgfx::createUniform("u_baseColor", bgfx::UniformType::Vec4);

	while (!ShouldWindowClose())
	{
		float deltaTime = StartFrame();

		// Camera control
		{
			const float camSpeed = 200.0f;
		
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


		Quatf rotation = Quatf::MakeFromEuler(cameraRot);
		Matrixf camera = Matrixf::MakeLookAt(rotation.GetForwardVector(), rotation.GetUpVector()) * Matrixf::MakeTranslation(cameraPos);
		Matrixf project = Matrixf::Perspective((float)width, (float)height, 1.f, 100000.0f, 60.0f);

		Vec4f lightDir(0.0f, -2.5f, -1.6f, 1.0f);

		bgfx::setViewTransform(0, &camera, &project);
		bgfx::setUniform(rState.m_lightDirectionUniform, &lightDir);

		RenderScene(plane, rState);
		RenderScene(terrain, rState);

		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(10, 10, 0x0f, "Hello world");
		
		EndFrame();
	}

	CloseWindow();

	return 0;
}