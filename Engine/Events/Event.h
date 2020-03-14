#pragma once

#include "PCH.h"

// TODO: Make this thread safe so we can listen for events on other threads
namespace Bat
{
	// erases event type and does a conversion back when called
	template <typename Event>
	struct EventCallbackWrapper
	{
		explicit EventCallbackWrapper(std::function<void(const Event&)> callback)
			:
			callback(callback)
		{}
		void operator()(const void* event)
		{
			callback(*(static_cast<const Event*>(event)));
		}
		std::function<void(const Event&)> callback;
	};

	class EventDispatcher
	{
	public:
		// for classes that want to be able to both listen/unlisten
		template <typename Event, typename Listener>
		void AddEventListener( Listener& listener )
		{
			void (Listener::*listen_func)(const Event&) = &Listener::OnEvent;
			ASSERT( !IsListeningForEvent<Event>( listener ), "Attempted to listen to same event multiple times" );
			auto wrapper = EventCallbackWrapper<Event>(std::bind(listen_func, &listener, std::placeholders::_1));
			m_Callbacks[std::type_index(typeid(Event))].emplace_back( &listener, wrapper );
		}
		template <typename Event, typename Listener>
		bool RemoveEventListener( Listener& listener )
		{
			auto& listeners = m_Callbacks[std::type_index(typeid(Event))];

			for( size_t i = 0; i < listeners.size(); i++ )
			{
				if( listeners[i].address == &listener )
				{
					listeners.erase( listeners.begin() + i );
					return true;
				}
			}

			return false;
		}
		template <typename Event, typename Listener>
		bool IsListeningForEvent( Listener& listener )
		{
			auto& listeners = m_Callbacks[std::type_index(typeid(Event))];

			for( size_t i = 0; i < listeners.size(); i++ )
			{
				if( listeners[i].address == &listener )
				{
					return true;
				}
			}

			return false;
		}

		// for classes that want to be able to both listen/unlisten
		template <typename Event, typename Listener>
		static void AddGlobalEventListener( Listener& listener )
		{
			void (Listener:: * listen_func)(const Event&) = &Listener::OnEvent;
			ASSERT( !IsListeningForGlobalEvent<Event>( listener ), "Attempted to listen to same event multiple times" );
			auto wrapper = EventCallbackWrapper<Event>( std::bind( listen_func, &listener, std::placeholders::_1 ) );
			m_GlobalCallbacks[std::type_index( typeid(Event) )].emplace_back( &listener, wrapper );
		}
		template <typename Event, typename Listener>
		static bool RemoveGlobalEventListener( Listener& listener )
		{
			auto& listeners = m_GlobalCallbacks[std::type_index( typeid(Event) )];

			for( size_t i = 0; i < listeners.size(); i++ )
			{
				if( listeners[i].address == &listener )
				{
					listeners.erase( listeners.begin() + i );
					return true;
				}
			}

			return false;
		}
		template <typename Event, typename Listener>
		static bool IsListeningForGlobalEvent( Listener & listener )
		{
			auto& listeners = m_GlobalCallbacks[std::type_index( typeid(Event) )];

			for( size_t i = 0; i < listeners.size(); i++ )
			{
				if( listeners[i].address == &listener )
				{
					return true;
				}
			}

			return false;
		}
		
		// for quick and dirty callbacks that dont need to be able to be removed
		template <typename Event, typename Listener>
		void OnEventDispatched( Listener& listener )
		{
			EventCallbackWrapper<Event> wrapper( listener );
			m_Callbacks[std::type_index(typeid(Event))].emplace_back( nullptr, wrapper );
		}
		template <typename Event, typename Listener>
		static void OnGlobalEventDispatched( Listener& listener )
		{
			EventCallbackWrapper<Event> wrapper( listener );
			m_GlobalCallbacks[std::type_index( typeid(Event) )].emplace_back( nullptr, wrapper );
		}

		template <typename Event, typename... Args>
		static void DispatchGlobalEvent( Args&& ... args )
		{
			const auto& listeners = m_GlobalCallbacks[std::type_index( typeid(Event) )];
			if( listeners.empty() )
			{
				return;
			}

			Event e = Event{ std::forward<Args>( args )... };
			for( auto listener : listeners )
			{
				listener.callback( &e );
			}
		}
	protected:
		template <typename Event, typename... Args>
		void DispatchEvent( Args&&... args )
		{
			const auto& listeners = m_Callbacks[std::type_index(typeid(Event))];
			if( listeners.empty() )
			{
				return;
			}

			Event e = Event{ std::forward<Args>( args )... };
			for( auto listener : listeners )
			{
				listener.callback( &e );
			}
		}
	private:
		struct EventListener
		{
			EventListener( void* pAddress, std::function<void( const void* )> cb )
				:
				address( pAddress ),
				callback( cb )
			{}

			void* address;
			std::function<void( const void* )> callback;
		};

		std::unordered_map<std::type_index, std::vector<EventListener>> m_Callbacks{ 20 };
		static std::unordered_map<std::type_index, std::vector<EventListener>> m_GlobalCallbacks;
	};
}