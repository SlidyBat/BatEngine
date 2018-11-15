#pragma once

namespace Bat
{
	class Window;
	class Graphics;
	class Model;

	class IScene
	{
	public:
		IScene( Window& wnd )
			:
			wnd( wnd )
		{}

		virtual void OnUpdate( float deltatime ) = 0;
		virtual void OnRender() = 0;
	protected:
		Window& wnd;
	};
}