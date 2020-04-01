#pragma once

namespace Bat
{
	class IApplication
	{
	public:
		virtual ~IApplication() = default;

		virtual void OnUpdate( float deltatime ) {};
		virtual void OnRender() {};
	};
}