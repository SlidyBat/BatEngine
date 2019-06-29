#pragma once

#include "IGPUDevice.h"
#include "ResourceManager.h"
#include "Entity.h"
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

	class Graphics
	{
	public:
		Graphics( Window& wnd );
		Graphics( const Graphics& src ) = delete;
		Graphics& operator=( const Graphics& src ) = delete;
		Graphics( Graphics&& donor ) = delete;
		Graphics& operator=( Graphics&& donor ) = delete;

		~Graphics();

		void Resize( int width, int height );

		SceneNode* GetActiveScene() const { return m_pSceneGraph; }
		void SetActiveScene( SceneNode* pSceneGraph ) { m_pSceneGraph = pSceneGraph; }

		Camera* GetActiveCamera() const { return m_pCamera; }
		void SetActiveCamera( Camera* pCamera ) { m_pCamera = pCamera; }

		HtmlUI& UI() { return m_UI; }
		const HtmlUI& UI() const { return m_UI; }

		void SetRenderGraph( RenderGraph* graph );

		void BeginFrame();
		void EndFrame();

		DirectX::XMMATRIX GetOrthoMatrix() const;
	private:
		void RenderScene();
		void RenderUI();
		void RenderImGui();
	private:
		DirectX::XMMATRIX m_matOrtho;

		int m_iScreenWidth = InitialScreenWidth;
		int m_iScreenHeight = InitialScreenHeight;

		SceneNode* m_pSceneGraph;
		Camera* m_pCamera;

		RenderGraph* m_pRenderGraph = nullptr;

		// UI
		HtmlUI m_UI;
	public:
		static constexpr bool	FullScreen = false;
		static constexpr int	VSyncEnabled = false;
		static constexpr float	ScreenFar = 2000.0f;
		static constexpr float	ScreenNear = 0.1f;
		static constexpr float	FOV = 90.0f;
		static constexpr float  FOVRadians = FOV * (DirectX::XM_PI / 180.0f);

		// only used when not in fullscreen
		static constexpr int	InitialScreenWidth = 1600;
		static constexpr int	InitialScreenHeight = 900;
	};
}