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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Commando/bandwidthcheck.h                    $*
 *                                                                                             *
 *                      $Author:: Bhayes                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/18/02 9:33p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef _BANDWIDTHCHECK_H
#define _BANDWIDTHCHECK_H

#include <WWOnline\WaitCondition.h>
#include <wwlib\except.h>
#include <windows.h>
#include <BandTest\BandTest.h>


namespace WWOnline {
	class Session;
}



class BandwidthCheckerClass
{

	public:

		/*
		** Struct for packing bandwidth levels into a single byte.
		*/
		#pragma pack(push)
		#pragma pack(1)
		typedef struct tInternalPackedBandwidthType {
			unsigned char Up		: 4;
			unsigned char Down	: 4;
		} InternalPackedBandwidthType;

		typedef union tPackedBandwidthType {
			InternalPackedBandwidthType Bandwidth;
			unsigned char RawBandwidth;
		} PackedBandwidthType;
		#pragma pack(pop)

		static RefPtr<WaitCondition> Detect(void);
		static void Check_Now(HANDLE event);

		static bool Got_Bandwidth(void) {return(GotBandwidth);};
		static void Force_Upstream_Bandwidth(unsigned int up);
		static unsigned long Get_Upstream_Bandwidth(void);
		static unsigned long Get_Reported_Upstream_Bandwidth(void);
		static wchar_t *Get_Upstream_Bandwidth_As_String(void);
		static unsigned long Get_Downstream_Bandwidth(void);
		static unsigned long Get_Reported_Downstream_Bandwidth(void);
		static wchar_t *Get_Downstream_Bandwidth_As_String(void);
		static wchar_t *Get_Bandwidth_As_String(void);
		static wchar_t *Get_Bandwidth_As_String(PackedBandwidthType bandwidth);
		static PackedBandwidthType Get_Packed_Bandwidth(void);
		static bool Failed_Due_To_No_Connection(void) {return(FailureCode == BANDTEST_NO_IP_DETECT);}
		static void Get_Compact_Log(StringClass &log_string);
		static bool Is_Thread_Running(void) { return Thread.Is_Running(); }

	private:

		static void Check(void);
		static const char *Get_Ping_Server_Name(void);

		static class BandwidthCheckerThreadClass : public ThreadClass {
			public:
				BandwidthCheckerThreadClass(const char *thread_name = "Bandwidth checker thread") : ThreadClass(thread_name, &Exception_Handler) {}
				void Thread_Function(void) {BandwidthCheckerClass::Check();};
		} Thread;
		friend BandwidthCheckerThreadClass;

		static HANDLE EventNotify;
		static unsigned long UpstreamBandwidth;
		static unsigned long ReportedUpstreamBandwidth;
		static unsigned long DownstreamBandwidth;
		static unsigned long ReportedDownstreamBandwidth;
		static wchar_t *UpstreamBandwidthString;
		static wchar_t *DownstreamBandwidthString;

		#define NUM_BANDS 12

		static char *ErrorList[13];
		static unsigned long Bandwidths[NUM_BANDS * 2];
		static wchar_t *BandwidthNames[NUM_BANDS + 1];
		static int FailureCode;
		static bool GotBandwidth;
		static const char *DefaultServerName;

};


/*
** Wait code for bandwidth detection.
**
**
**
*/
class BandwidthDetectWait : public SingleWait
{
	public:
		static RefPtr<BandwidthDetectWait> Create(void);

		void WaitBeginning(void);
		WaitResult GetResult(void);


	protected:
		BandwidthDetectWait();
		virtual ~BandwidthDetectWait();

		BandwidthDetectWait(const BandwidthDetectWait&);
		const BandwidthDetectWait& operator = (const BandwidthDetectWait&);

		RefPtr<WWOnline::Session> WOLSession;
		unsigned int mPingsRemaining;
		HANDLE mEvent;


};











#endif //_BANDWIDTHCHECK_H