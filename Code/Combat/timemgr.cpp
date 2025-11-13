/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/timemgr.cpp                           $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/14/02 11:29a                                              $*
 *                                                                                             *
 *                    $Revision:: 64                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "timemgr.h"
#include "always.h"
#include "ww3d.h"
#include "debug.h"
#include "slist.h"
#include "input.h"
#include "combat.h"
#include "ccamera.h"
#include "chunkio.h"
#include "persistfactory.h"
#include "combatchunkid.h"
#include "wwprofile.h"

#include "win.h"
//#include "systimer.h"		// for timegettime
#include "systimer.h"


/*
**
*/
int			TimeManager::FrameTicks = 0;
int			TimeManager::RealFrameTicks = 0;
int			TimeManager::LastTicks = 0;
float			TimeManager::TimeScale = 1.0f;
float			TimeManager::TotalSeconds = 0;
float			TimeManager::AveragedFPS = 0.0f;
int			TimeManager::AveragedFPSTicks = 0;
int			TimeManager::AveragedFPSCounter = 0;
float			TimeManager::FrameSeconds = 0.0f;
float			TimeManager::RealFrameSeconds = 0.0f;

#define		TIME_STABILIZING_TECHNOLOGY_ENABLED			0
#define		TIME_DESTABILIZING_TECHNOLOGY_ENABLED		0
#define		SLOWEST_FPS									      5

FrameTimeHistogramClass::FrameTimeHistogramClass(unsigned slot_count, float step)
	:
	SlotCount(slot_count),
	Step(step)
{
	Counts=new unsigned[SlotCount];
	Reset();
}

FrameTimeHistogramClass::~FrameTimeHistogramClass()
{
	delete[] Counts;
}

// Report normalized counts for each frame time slot, packed to unsigned bytes.
void FrameTimeHistogramClass::Get_Packed_Report(unsigned char* bytes)
{
	unsigned total=0;
	unsigned i;
	for (i=0;i<SlotCount;++i) {
		total+=Counts[i];
	}
	if (total==0) total=1;

	for (i=0;i<SlotCount;++i) {
		unsigned char value=255*Counts[i]/total;
		if (value==0 && Counts[i]!=0) value=1;
		bytes[i]=value;
	}

}

// Report the absolute counts for each frame time slot
void FrameTimeHistogramClass::Get_Report(unsigned* counts)
{
	unsigned i;
	for (i=0;i<SlotCount;++i) {
		counts[i]=Counts[i];
	}
}

// Reset the counts.
void FrameTimeHistogramClass::Reset()
{
	for (unsigned i=0;i<SlotCount;++i) {
		Counts[i]=0;
	}
}

// Place the time of current frame to a correct slot.
void FrameTimeHistogramClass::Add(float frame_time)
{
	unsigned int slot=WWMath::Float_To_Long(frame_time*(1000.0f/Step));
	if (slot>=SlotCount) slot=SlotCount-1;
	Counts[slot]++;
}


FrameTimeHistogramClass FrameTimeHistogram(16,15.0f);

FrameTimeHistogramClass& TimeManager::Peek_Frame_Time_Histogram()
{
	return FrameTimeHistogram;
}


/*
**
*/
int	TimeManager::SystemTicks()
{
	return TIMEGETTIME();
}

void TimeManager::Reset(void)
{
	FrameTicks = 0;
	RealFrameTicks = 0;
	LastTicks = 0;
	TimeScale = 1.0f;
	TotalSeconds = 0;
	AveragedFPS = 0.0f;
	AveragedFPSTicks = 0;
	AveragedFPSCounter = 0;
	FrameSeconds = 0.0f;
	RealFrameSeconds = 0.0f;
}


/*
**
*/
void	TimeManager::Update_Frame_Time()
{
	int	ticks = SystemTicks();
	if ( LastTicks == 0) {		// sync first time
		LastTicks = ticks;
	}

	FrameTicks = ticks - LastTicks;
	RealFrameTicks = ticks - LastTicks;
	LastTicks = ticks;

	FrameTicks = MIN( FrameTicks, (TICKS_PER_SECOND / SLOWEST_FPS) );

	if ( WW3D::Get_Movie_Capture_Frame_Rate() != 0.0f ) {
		FrameTicks = TICKS_PER_SECOND / WW3D::Get_Movie_Capture_Frame_Rate();
	}

#if	TIME_STABILIZING_TECHNOLOGY_ENABLED
	FrameTicks = TICKS_PER_SECOND / 30;
#endif

#if	TIME_DESTABILIZING_TECHNOLOGY_ENABLED
	static int state = 0;
	FrameTicks = TICKS_PER_SECOND / 15;
	if ( ++state & 0x2 ) {
	   FrameTicks = TICKS_PER_SECOND / 30;
	}
#endif

	// Single Step
	static bool single_step = false;

#ifdef WWDEBUG
	if ( Input::Get_State( INPUT_FUNCTION_DEBUG_SINGLE_STEP ) ) {
		single_step = !single_step;
		Debug_Say(( "Single_Step %s\n", single_step ? "ON" : "OFF" ));
	}

	if ( single_step ) {

//		Wait_Seconds( 0.2 );

		FrameTicks = 0;
		if ( Input::Get_State( INPUT_FUNCTION_DEBUG_SINGLE_STEP_STEP ) ) {
			Debug_Say(( "Single_Step STEP!!\n" ));
			FrameTicks = TICKS_PER_SECOND / 15;
		}
	}
#endif

	if ( COMBAT_CAMERA && COMBAT_CAMERA->Is_Snap_Shot_Mode() ) {
		FrameTicks = 0;
	}

	if ( CombatManager::Is_Game_Paused() ) {
		FrameTicks = 0;
	}

	FrameTicks *= TimeScale;
	FrameSeconds=(float)FrameTicks / TICKS_PER_SECOND;
	RealFrameSeconds=(float)RealFrameTicks / TICKS_PER_SECOND;

	WW3D::Sync( WW3D::Get_Sync_Time() + FrameTicks );

/*
#if 0
	// testing animation smoothing
	if (!WWDEBUG_TRIGGER(WWDEBUG_TRIGGER_GENERIC0) ) {
		static int rlticks = 0;

		if (rlticks == 0) {
			rlticks = ticks;
		}
		int rtime = ticks - rlticks;

		//float amount = (float)rtime / (3.0f*1000.0f);
//Debug_Say(( "Time  %d %f\n", rtime, amount ));
		if (rtime < 200) {
//			Wait_Seconds( amount );
		}
		rlticks = SystemTicks();
	}
#endif

#if 0
	//if ( IS_SOLOPLAY && Input::Get_State( INPUT_FUNCTION_DEBUG_RAPID_MOVE ) ) {
	if ( Input::Get_State( INPUT_FUNCTION_DEBUG_RAPID_MOVE ) ) {
		if ( COMBAT_CAMERA->Is_Using_Host_Model() ) {
			FrameTicks *= 10;
//			FrameTicks /= 5;
		}
	}
#endif

#if 0
	if (FrameTicks == 0) {
//		WWDEBUG_SAY(("Frame Ticks was ZERO!\n"));
		FrameTicks = 1;
	}
#endif
*/
	TotalSeconds += Get_Frame_Seconds();

	/*
	** Time averaged fps (averaged over 10 seconds)
	*/
	AveragedFPSTicks += RealFrameTicks;
	AveragedFPSCounter++;
	if ( AveragedFPSTicks >= 10 * TICKS_PER_SECOND ) {
		AveragedFPS = (float)AveragedFPSCounter / ((float)AveragedFPSTicks / (float)TICKS_PER_SECOND);
		AveragedFPSTicks = AveragedFPSCounter = 0;
	}

	FrameTimeHistogram.Add(FrameSeconds);
}

void	TimeManager::Wait_Seconds( float time )
{
	float end = Get_Seconds() + time;
	while ( Get_Seconds() < end );
}

/***********************************************************************************************
 * TimeManager::Update( void ) - updates all current timers by the frame time
 *                                                                                             *
 * INPUT:
 * 																														  *
 * OUTPUT:
 * 																														  *
 * WARNINGS:	None																									  *
 * 																														  *
 * HISTORY:                                                                                    *
 *   09/01/1997 BG  : Created.                                                                 *
 *=============================================================================================*/
void	TimeManager::Update( void )
{
	Update_Frame_Time();

	// tell the profiling code that another frame has gone by
	WWProfileManager::Increment_Frame_Counter();


#ifdef WWDEBUG
	//
	// TSS101401
	// Watch out for unexpected slow frames. They may interrupt networking.
	//
	if (Get_Frame_Real_Seconds() > 2) {
		Debug_Say(("TimeManager::Update: warning, frame %d was slow (%d ms)\n",
			WWProfileManager::Get_Frame_Count_Since_Reset(),
			(int)(1000 * Get_Frame_Real_Seconds())));
	}
#endif // WWDEBUG
}


