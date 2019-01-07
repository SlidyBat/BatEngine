#pragma once

#include <Ultralight/Ultralight.h>
#include "UI/UltralightAdapters/PlatformD3D11.h"

#include "MathLib.h"
#include "PoolAllocator.h"
#include "FileWatchdog.h"

namespace Bat
{
	struct WindowResizeEvent;
	class Window;

	class Overlay
	{
	public:
		Overlay(ultralight::Ref<ultralight::Renderer> renderer, ultralight::GPUDriver* driver,
			Window& wnd, int width, int height, const Vei2& pos, float scale);

		virtual ~Overlay();

		void LoadHTMLRaw( const std::string& html );
		void LoadHTMLFromFile( const std::string& filename );
		void LoadHTMLFromURL( const std::string& filename );

		int Width() const { return m_iWidth; }
		int Height() const { return m_iHeight; }
		Vei2 GetPos() const { return m_vecPos; }

		void MoveTo( const Vei2& pos ) { m_vecPos = pos; m_bDirty = true; }
		void MoveBy( const Vei2& pos ) { m_vecPos += pos; m_bDirty = true; }

		bool Contains(const Vei2& pos) const
		{
			return pos.x >= m_vecPos.x &&
				pos.y >= m_vecPos.y &&
				pos.x < m_vecPos.x + m_iWidth &&
				pos.y < m_vecPos.y + m_iHeight;
		}

		virtual void Draw();
		virtual void Resize(int width, int height);

		void OnEvent( const WindowResizeEvent& e );
	private:
		void UpdateGeometry();

		void OnFileChanged( const std::string& filename );
		void StartWatchingFile( const std::string& filename );
		void StopWatchingFile();
	private:
		friend class BatUI;

		int m_iWidth;
		int m_iHeight;
		Vei2 m_vecPos;
		float m_flScale;
		bool m_bHasFocus = false;
		bool m_bHasHover = false;
		ultralight::Ref<ultralight::View> m_pView;
		ultralight::GPUDriver* m_pDriver;
		std::vector<ultralight::Vertex_2f_4ub_2f_2f_28f> m_Vertices;
		std::vector<ultralight::IndexType> m_Indices;
		bool m_bDirty = true;
		uint32_t m_iGeometryId;
		ultralight::GPUState m_GPUState;

		FileListenerHandle_t m_iListenHandle = FileWatchdog::INVALID_LISTENER;
	};

	class BatUI
	{
	public:
		BatUI( Window& wnd );

		void Update();
		void Render();

		Overlay* CreateOverlay( int width, int height, const Vei2& pos, float scale = 1.0f );
		void DeleteOverlay( Overlay* pOverlay );
	private:
		void DrawOverlays();
	private:
		ultralight::Config config;
		std::unique_ptr<ultralight::GPUDriver>                    gpu_driver;
		std::unique_ptr<ultralight::FontLoader>                   font_loader;
		std::unique_ptr<ultralight::FileSystem>                   filesystem;
		std::unique_ptr<framework::PlatformGPUContextD3D11> gpu_context;
		ultralight::RefPtr<ultralight::Renderer>                  renderer;

		std::vector<Overlay*>                                     overlays;
		ObjectPoolAllocator<Overlay>                              overlay_allocator{ 100 };

		Window& wnd;
	};
};