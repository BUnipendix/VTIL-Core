#pragma once

// Local patch: scoped sub-deadline shared by the wall-clock guards.
//
// The routine-wide budget (NOVMP_TIME_LIMIT) cannot bound a single symbolic
// refinement inside a routine: on giant virtualized routines one backward
// trace through the whole block history diverges and eats the entire budget,
// discarding every already-lifted handler. Refinement call sites in the
// lifter arm a scoped sub-deadline before the trace; the guards in
// vtil_lifter.cpp / tracer.cpp / simplifier.cpp then fail fast once it
// passes, the site catches the "time budget exceeded" error and falls back
// to its default path.
//
#include <chrono>
#include <cstdlib>

namespace vtil::budget
{
	// The sub-deadline currently in effect (thread-local); max() when none.
	//
	inline std::chrono::steady_clock::time_point& sub_deadline()
	{
		thread_local static std::chrono::steady_clock::time_point t =
			std::chrono::steady_clock::time_point::max();
		return t;
	}

	// True once the scoped deadline has passed.
	//
	inline bool sub_expired()
	{
		return std::chrono::steady_clock::now() > sub_deadline();
	}

	// While alive, the guards also fail fast once the scoped deadline passes,
	// independently of the routine-wide deadline. Nested scopes never extend
	// an earlier deadline (min semantics) and restore it on unwind.
	//
	struct scoped_sub_deadline
	{
		bool armed;
		std::chrono::steady_clock::time_point prev{};
		scoped_sub_deadline( int64_t secs ) : armed( secs > 0 )
		{
			if ( armed )
			{
				prev = sub_deadline();
				std::chrono::steady_clock::time_point next =
					std::chrono::steady_clock::now() + std::chrono::seconds( secs );
				if ( next < prev )
					sub_deadline() = next;
			}
		}
		~scoped_sub_deadline()
		{
			if ( armed )
				sub_deadline() = prev;
		}
		scoped_sub_deadline( const scoped_sub_deadline& ) = delete;
		scoped_sub_deadline& operator=( const scoped_sub_deadline& ) = delete;
	};

	// Seconds a single bounded refinement may run before the guards fail it.
	// NOVMP_RTRACE_BUDGET overrides; <=0 disables bounding entirely.
	//
	inline int64_t refinement_budget_secs()
	{
		static const int64_t v = []
		{
			int64_t s = 10;
			if ( const char* e = getenv( "NOVMP_RTRACE_BUDGET" ) )
				s = atoll( e );
			return s;
		}();
		return v;
	}
}
