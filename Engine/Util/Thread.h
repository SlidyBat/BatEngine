#pragma once

#include "BatAssert.h"
#include "Mutex.h"
#include <future>

namespace Bat
{
	template <typename T>
	class Future
	{
	public:
		Future( std::future<T> future )
			:
			future( std::move( future ) )
		{}

		bool IsReady() const
		{
			return future.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready;
		}
		T Get()
		{
			ASSERT( future.valid(), "Attempted to get invalid future" );
			return future.get();
		}
		void Wait() const
		{
			future.wait();
		}
		// Returns true if result finished before the timeout period has passed, otherwise false
		template <typename Rep, typename Period>
		bool WaitFor( const std::chrono::duration<Rep, Period>& timeout ) const
		{
			return future.wait_for( timeout ) == std::future_status::ready;
		}
		// Returns true if result finished before the time point is reached, otherwise false
		template <typename Clock, typename Duration>
		bool WaitUntil( const std::chrono::time_point<Clock, Duration>& time ) const
		{
			return future.wait_until( time ) == std::future_status::ready;
		}
	private:
		std::future<T> future;
	};
}