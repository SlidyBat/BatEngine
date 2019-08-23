#pragma once

#include <string>
#include <unordered_map>
#include "IGPUDevice.h"
#include "Camera.h"

namespace Bat
{
	class Texture;
	class RenderTarget;

	class RenderData
	{
		friend class RenderGraph;
#define RENDER_NODE_DATATYPE( type, name, capname ) \
	public: \
		type* Get##name( const std::string& resource_name ); \
		void Add##name( const std::string& resource_name, type* pResource ); \
	private: \
		std::unordered_map<std::string, type*> m_map##name;
#include "RenderNodeDataTypes.def"
	};
}