#if !defined(_CALLSTACK_H)
#define _CALLSTACK_H

#include <map>
#include "Array.h"
#include "Timer.h"

void Callstack_InitModule();
void Callstack_DeinitModule();

#if !defined(_RETAIL)
	extern Array<char *> Callstack;

#if defined(_CALLSTACK_MONITORING)

	class CallProfile;

	class MonitorCall
	{
	public:
		MonitorCall(bool enter, CallProfile *ptr, uint64 time)
		: start(enter), timestamp(time) { profilePtr = ptr; }

		bool start;
		CallProfile *profilePtr;
		uint64 timestamp;
	};

	struct MonitorInfo
	{
		MonitorInfo() { frameStart = frameFinish = frameEnd = overheadTime = 0; frames = 1; }

		int64 frameStart;
		int64 frameFinish;
		int64 frameEnd;
		int64 overheadTime;
		Array<MonitorCall> calls;
		int frames;
		double divide;
	};

#endif

#if defined(_CALLSTACK_PROFILING)
	class CallProfile
	{
	public:
		CallProfile();

		int calls;

		int64 thisCall;
		int64 total;

#if defined(_CALLSTACK_MONITORING)
		uint32 colour;
#endif
	};
#endif

	extern std::map<char *, CallProfile> FunctionRegistry;

	class FunctionCall
	{
	public:
		inline FunctionCall(char *name, int profile)
		{
			Callstack.push() = name;

#if defined(_CALLSTACK_PROFILING)
			profiling = false;

			if(profile)
			{
				profiling = true;

				pProfile = &FunctionRegistry[name];
				pProfile->calls++;

				int64 temp;
				temp = GetHighResolutionTime();

				pProfile->thisCall = temp;

#if defined(_CALLSTACK_MONITORING)
				monitorInfo.calls.push_back(MonitorCall(true, &t, temp));
#endif
			}
#endif
		}

		inline ~FunctionCall()
		{
#if defined(_CALLSTACK_PROFILING)
			if(profiling)
			{
				int64 temp;
				temp = GetHighResolutionTime();

#if defined(_CALLSTACK_MONITORING)
				monitorInfo.calls.push_back(MonitorCall(false, &t, temp));
#endif

				pProfile->total += temp-pProfile->thisCall;
			}
#endif

			Callstack.pop();
		}

#if defined(_CALLSTACK_PROFILING)
		bool profiling;
		CallProfile *pProfile;
#endif
	};

#if defined(_CALLSTACK_PROFILING)
	void Callstack_BeginFrame();

	#if defined(_CALLSTACK_MONITORING)
		void Callstack_EndFrame();
	#else
		#define Callstack_EndFrame()
	#endif

	#define CALLSTACK FunctionCall callstack(__FUNCTION__, 0);
	#define CALLSTACKc FunctionCall callstack(__FUNCTION__, 1);
	#define CALLSTACKs(s) FunctionCall callstack(s, 0);
	#define CALLSTACKcs(s) FunctionCall callstack(s, 1);

	void Callstack_DrawProfile();
#else
	#define Callstack_BeginFrame()
	#define CALLSTACK
	#define CALLSTACKc
	#define CALLSTACKs(s)
	#define CALLSTACKcs(s)
	#define Callstack_DrawProfile()
	#define Callstack_EndFrame()
#endif

#else

	#define Callstack_BeginFrame()
	#define CALLSTACK
	#define CALLSTACKc
	#define CALLSTACKs(s)
	#define CALLSTACKcs(s)
	#define Callstack_DrawProfile()
	#define Callstack_EndFrame()

#endif // _RETAIL

#endif // _CALLSTACK_H
