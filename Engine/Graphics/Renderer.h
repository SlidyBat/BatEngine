#pragma once

#include "IGPUDevice.h"
#include "Core/ResourceManager.h"
#include "Core/Entity.h"
#include "UI/HtmlUI.h"

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Bat
{
	class Window;
	class SceneGraph;
	class Camera;
	class RenderGraph;
	class SceneNode;

	class Renderer
	{
	public:
		Renderer( Window& wnd );
		Renderer( const Renderer& src ) = delete;
		Renderer& operator=( const Renderer& src ) = delete;
		Renderer( Renderer&& donor ) = delete;
		Renderer& operator=( Renderer&& donor ) = delete;

		~Renderer();

		void Resize( size_t width, size_t height );

		SceneNode* GetActiveScene() const { return m_pSceneGraph; }
		void SetActiveScene( SceneNode* pSceneGraph ) { m_pSceneGraph = pSceneGraph; }

		Camera* GetActiveCamera() const { return m_pCamera; }
		void SetActiveCamera( Camera* pCamera ) { m_pCamera = pCamera; }

		HtmlUI& UI() { return m_UI; }
		const HtmlUI& UI() const { return m_UI; }

		void SetRenderGraph( RenderGraph* graph );

		void BeginFrame();
		void EndFrame();

		Mat4 GetOrthoMatrix() const;
	private:
		void InitialiseResources( size_t width, size_t height );

		void RenderScene();
		void RenderUI();
		void RenderImGui();
	private:
		Mat4 m_matOrtho;

		int m_iScreenWidth = 0;
		int m_iScreenHeight = 0;

		SceneNode* m_pSceneGraph;
		Camera* m_pCamera;

		RenderGraph* m_pRenderGraph = nullptr;

		// UI
		HtmlUI m_UI;
	public:
		static constexpr bool	FullScreen = false;
		static constexpr int	VSyncEnabled = false;
		static constexpr float	ScreenFar = 200.0f;
		static constexpr float	ScreenNear = 0.1f;
		static constexpr float	FOV = 90.0f;
		static constexpr float  FOVRadians = Math::DegToRad( FOV );

		// only used when not in fullscreen
		static constexpr int	InitialScreenWidth = 1366;
		static constexpr int	InitialScreenHeight = 768;
	};
}