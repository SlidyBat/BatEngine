#pragma once

namespace Bat
{
	class ILayer
	{
	public:
		virtual ~ILayer() = default;

		virtual void OnUpdate( float deltatime ) {};
		virtual void OnRender() {};
	};
}