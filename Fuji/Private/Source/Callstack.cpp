/**** Defines ****/


/**** Includes ****/

#include "Common.h"
#include "Display.h"
#include "Timer.h"
#include "Primitive.h"
#include "System.h"
#include "Font.h"
#include "DebugMenu.h"

#if defined(_CALLSTACK_PROFILING)
MenuItemBool drawCallstack(false);
#endif

void Callstack_InitModule()
{
#if defined(_CALLSTACK_PROFILING)
	DebugMenu_AddItem("Draw Callstack", "Fuji Options", &drawCallstack);
#endif
}

void Callstack_DeinitModule()
{

}

#if !defined(_RETAIL)

/**** Globals ****/

std::vector<std::string> Callstack;

#if defined(_CALLSTACK_PROFILING)
std::map<std::string, CallProfile> FunctionRegistry;
#endif

#if defined(_CALLSTACK_MONITORING)

	MonitorInfo monitorInfo;

	// cyan - overhead, red - used time, blue - waiting time

	static uint32 callstackColours[] = {
		/* no red */0xFFFFFF00, 0xFF00FF00, 0xFFFF00FF, 0xFF009DFF, 0xFFAA00FF, 0xFF00FFAE, 0xFF0000FF, 0xFFFFA200, 0xFF00FFFF, 
		0xFF800000, 0xFF808000, 0xFF008000, 0xFF800080, 0xFF004F80, 0xFF550080, 0xFF008057, 0xFF000080, 0xFF805100, 0xFF008080, // dark
		0xFFFF8080, 0xFFFFFF80, 0xFF80FF80, 0xFFFF80FF, 0xFF80CEFF, 0xFFD480FF, 0xFF80FFD7, 0xFF8080FF, 0xFFFFD080, 0xFF80FFFF  // light
	};

	static uint32 assignedColours = -1;

	struct MonitorDisplayList
	{
		MonitorDisplayList(double _start, uint32 col) : start(_start), colour(col) {}

		double start, end;
		uint32 colour;
	};

	static double monitorDispEnd, monitorDispFinish, monitorDispOverhead;

	static std::vector<MonitorDisplayList> displayList;

#endif

/**** Functions ****/

#if defined(_CALLSTACK_MONITORING)

void Callstack_PrepareMonitoredData()
{
	CALLSTACK("Callstack_PrepareMonitoredData");

	if(!monitorInfo.overheadTime) return;

	displayList.clear();

	uint64 frameTime = monitorInfo.frameEnd-monitorInfo.frameStart;	// total time spent in this frame
	uint64 frameLength = GetHighResolutionFrequency()/60;			// 60th of second

	double frames = (double)frameTime / (double)frameLength;		// number of 6th's of second

	monitorInfo.frames = (int)ceil(frames);							// number of 60th's as an int

	uint64 minGap = (frameLength / 620)*monitorInfo.frames*2 - 1;	// time in one pixel on meter

	monitorInfo.divide = (double)frameLength * (double)monitorInfo.frames;

	monitorDispFinish = (double)(monitorInfo.frameFinish-monitorInfo.frameStart) / monitorInfo.divide;
	monitorDispEnd = (double)(monitorInfo.frameEnd-monitorInfo.frameStart) / monitorInfo.divide;
	monitorDispOverhead = (double)(monitorInfo.overheadTime) / monitorInfo.divide;

	int a, b, c, push;
	int callCount = monitorInfo.calls.size();

	for(a=0; a<callCount; a++)
	{
		if(monitorInfo.calls[a].start)
		{
			push = 0;

			displayList.push_back(MonitorDisplayList((double)(monitorInfo.calls[a].timestamp-monitorInfo.frameStart)/monitorInfo.divide, monitorInfo.calls[a].profilePtr->colour));

			for(b=a+1; b<callCount; b++)
			{
				if(monitorInfo.calls[b].profilePtr == monitorInfo.calls[a].profilePtr && monitorInfo.calls[b].start)
					push++;

				if(monitorInfo.calls[b].profilePtr == monitorInfo.calls[a].profilePtr && !monitorInfo.calls[b].start)
				{
					if(push) push--;
					else
					{
						for(c=b+1; c<callCount; c++)
						{
							if(monitorInfo.calls[c].profilePtr == monitorInfo.calls[a].profilePtr)
							{
								if(monitorInfo.calls[c].start && monitorInfo.calls[c].timestamp-monitorInfo.calls[b].timestamp < minGap) monitorInfo.calls[c].start = false;
								else c = callCount;

								break;
							}
						}

						if(c<callCount)
						{
							b = c;
						}
						else
						{
							displayList.back().end = (double)(monitorInfo.calls[b].timestamp-monitorInfo.frameStart)/monitorInfo.divide;
							break;
						}
					}
				}
			}
		}
	}
}
#endif

#if defined(_CALLSTACK_PROFILING)

void DrawUntexturedSprite(float x, float y, float w, float h, uint32 col)
{
	MFPrimitive(PT_TriStrip|PT_Untextured);

	MFBegin(4);

	MFSetColour(col);
	MFSetPosition(x, y, 0);
	MFSetPosition(x+w, y, 0);
	MFSetPosition(x, y+h, 0);
	MFSetPosition(x+w, y+h, 0);
	MFEnd();
}

void Callstack_DrawProfile()
{
	CALLSTACK("Callstack_DrawProfile");

	bool t = SetOrtho(true);

	int a;

#if defined(_CALLSTACK_MONITORING)
	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&Matrix::identity);
//	View::current.SetLocalToWorld(Matrix::identity);

	float inc, across;
	float width = 396;

	if(monitorInfo.frames<100)
	{
		MFPrimitive(PT_LineList | PT_Prelit | PT_Untextured);
		MFBegin(2 + monitorInfo.frames*2);
		MFSetColour(0xFFFFFF00);

		MFSetPosition(42, 398, 0);
		MFSetPosition(42, 436, 0);

		inc = width / (float)monitorInfo.frames;
		across = 41 + inc;
		for(a=0; a<monitorInfo.frames; a++)
		{
			MFSetPosition(across, 398, 0);
			MFSetPosition(across, 436, 0);
			across += inc;
		}

		MFEnd();
	}

	DrawUntexturedSprite(40, 400, 400, 34, 0xFFC0C0C0);
	DrawUntexturedSprite(42, 402, 396, 30, 0xFF808080);

	DrawUntexturedSprite(42, 402, float(monitorDispFinish)*width, 30, 0xFFFF0000);
	DrawUntexturedSprite(42 + float(monitorDispFinish)*width, 402, float(monitorDispEnd-monitorDispFinish)*width, 30, 0xFF0000FF);
//	DrawUntexturedSprite(10 + float(monitorDispEnd)*width, 10, float(monitorDispFinish)*width + float(monitorDispOverhead)*width, 30, 0xFF00FFFF);

	for(a=0; a<displayList.size(); a++)
	{
		if(a ? (displayList[a].colour != displayList[a-1].colour && displayList[a].start < displayList[a-1].end) : 1)
		{
			DrawUntexturedSprite(42 + float(displayList[a].start)*width, 402, float(displayList[a].end-displayList[a].start)*width, 30, displayList[a].colour);
		}
	}
#endif

	if(drawCallstack)
	{
		DrawUntexturedSprite(90, 90, 460, 320, 0xB0000000);

		std::map<std::string, CallProfile>::iterator i;
		float y = 100;

		for(i = FunctionRegistry.begin(), a=0; i!= FunctionRegistry.end() && a<19; i++, a++)
		{
			uint32 ms = (uint32)(i->second.total / (GetHighResolutionFrequency()/1000000));
			double percent = (double)i->second.total/((double)GetHighResolutionFrequency() * 0.01/60.0);

			debugFont.DrawTextf(100, y, 0, 15.0f, 0xFFFFFFFF, "%s()", i->first.c_str());
			debugFont.DrawTextf(300, y, 0, 15.0f, 0xFFFFFFFF, "%d�s - %.2f%% - %d calls", ms, percent, i->second.calls);
			y += 15.0f;
		}

		if(a = 19)
			debugFont.DrawTextf(110, y, 0, 15.0f, 0xFFFFFF, "More...");
	}

	SetOrtho(t);
}
#endif

#if defined(_CALLSTACK_PROFILING)
void Callstack_BeginFrame()
{
	CALLSTACK("Callstack_BeginFrame");

#if defined(_CALLSTACK_MONITORING)
	monitorInfo.frameEnd = GetHighResolutionTime();

	Callstack_PrepareMonitoredData();

	monitorInfo.overheadTime = monitorInfo.frameEnd - monitorInfo.frameStart;

	monitorInfo.calls.clear();
#endif

	std::map<std::string, CallProfile>::iterator i;

	for(i=FunctionRegistry.begin(); i!=FunctionRegistry.end(); i++)
	{
		i->second.calls = 0;
		i->second.total = 0;

#if defined(_CALLSTACK_MONITORING)
		if(!i->second.colour) i->second.colour = callstackColours[assignedColours = (assignedColours+1) % (sizeof(callstackColours)/4)];
#endif
	}

#if defined(_CALLSTACK_MONITORING)
	monitorInfo.frameStart = GetHighResolutionTime();
	monitorInfo.overheadTime += monitorInfo.frameStart - monitorInfo.frameEnd;
#endif
}
#endif

#if defined(_CALLSTACK_MONITORING)
void Callstack_EndFrame()
{
	monitorInfo.frameFinish = GetHighResolutionTime();
}
#endif

FunctionCall::FunctionCall(char *name, int profile)
{
	Callstack.push_back(name);

#if defined(_CALLSTACK_PROFILING)
	profiling = false;

	if(profile)
	{
		int64 temp = GetHighResolutionTime();

		profiling = true;

		CallProfile &t = FunctionRegistry[name];

		t.calls++;
		t.thisCall = temp;

#if defined(_CALLSTACK_MONITORING)
		monitorInfo.calls.push_back(MonitorCall(true, &t, temp));
#endif
	}
#endif
}

FunctionCall::~FunctionCall()
{
#if defined(_CALLSTACK_PROFILING)
	if(profiling)
	{
		int64 temp;
		temp = GetHighResolutionTime();

		CallProfile &t = FunctionRegistry[Callstack.back()];

#if defined(_CALLSTACK_MONITORING)
		monitorInfo.calls.push_back(MonitorCall(false, &t, temp));
#endif

		t.total += temp-t.thisCall;
	}
#endif

	Callstack.pop_back();
}

#if defined(_CALLSTACK_PROFILING)
CallProfile::CallProfile()
{
	calls = 0;
	total = 0;
#if defined(_CALLSTACK_MONITORING)
	colour = 0;
#endif
}
#endif

#endif // _RETAIL
