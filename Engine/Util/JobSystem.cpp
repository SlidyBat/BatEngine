#include "JobSystem.h"

#include <algorithm>    // std::max
#include <future>

#include "BatAssert.h"

namespace Bat
{
	static std::vector<std::future<void>> g_ThreadPromises;

	void JobSystem::Initialize()
	{
	}

	void JobSystem::Shutdown()
	{
	}

	void JobSystem::Execute(const std::function<void()>& job)
	{
		g_ThreadPromises.emplace_back(
			std::async( std::launch::async, job )
		);
	}

	bool JobSystem::IsBusy()
	{
		return !g_ThreadPromises.empty();
	}

	void JobSystem::Wait()
	{
		for( const auto& promise : g_ThreadPromises )
		{
			promise.wait();
		}

		g_ThreadPromises.clear();
	}

	void JobSystem::Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(JobDispatchArgs)>& job)
	{
		ASSERT( !(jobCount == 0 || groupSize == 0), "Invalid parameters" );

		// Calculate the amount of job groups to dispatch (overestimate, or "ceil"):
		const uint32_t groupCount = (jobCount + groupSize - 1) / groupSize;

		for (uint32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex)
		{
			// For each group, generate one real job:
			auto& jobGroup = [jobCount, groupSize, job, groupIndex]()
			{
				// Calculate the current group's offset into the jobs:
				const uint32_t groupJobOffset = groupIndex * groupSize;
				const uint32_t groupJobEnd = std::min(groupJobOffset + groupSize, jobCount);

				JobDispatchArgs args;
				args.groupIndex = groupIndex;

				// Inside the group, loop through all job indices and execute job for each index:
				for (uint32_t i = groupJobOffset; i < groupJobEnd; ++i)
				{
					args.jobIndex = i;
					job(args);
				}
			};

			g_ThreadPromises.emplace_back(
				std::async( std::launch::async, jobGroup )
			);
		}
	}
}