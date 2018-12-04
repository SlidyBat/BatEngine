#pragma once

#include <string_view>
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include "BatAssert.h"

#define ON_EVENT_DISPATCHED( cb ) Bat::EventManager::AddListener( std::function( cb ) )
#define DISPATCH_EVENT( e ) Bat::EventManager::Dispatch( e )

namespace Bat
{
	class IEvent
	{
	public:
		virtual ~IEvent() = default;
	};


	class EventManager
	{
	public:
		template <typename T>
		static void AddListener( std::function<void(const T*)> callback )
		{
			s_mapCallbacks[std::type_index( typeid(T) )].emplace_back( [=]( const IEvent* args ) { callback( (const T*)args ); } );
		}

		template <typename T>
		static void Dispatch( const T& e )
		{
			auto it = s_mapCallbacks.find( std::type_index( typeid(T) ) );
			if( it != s_mapCallbacks.end() )
			{
				const auto& callbacks = it->second;
				for( const auto& cb : callbacks )
				{
					cb( &e );
				}
			}
		}
	private:
		static std::unordered_map<std::type_index, std::vector<std::function<void( const IEvent* )>>> s_mapCallbacks;
	};
}