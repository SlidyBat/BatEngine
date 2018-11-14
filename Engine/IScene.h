#pragma once

namespace Bat
{
	class Window;
	class Graphics;
	class Model;

	class IScene
	{
	public:
		IScene( Window& wnd, Graphics& gfx )
			:
			wnd( wnd ),
			gfx( gfx )
		{}

		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
	protected:
		Window& wnd;
		Graphics& gfx;
	};
}