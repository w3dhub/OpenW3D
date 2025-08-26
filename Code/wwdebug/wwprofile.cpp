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
 *                 Project Name : WWDebug                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwdebug/wwprofile.cpp                        $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/01/02 10:30a                                              $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *   WWProfile_Get_Ticks -- Retrieves the cpu performance counter                              *
 *   WWProfileHierachyNodeClass::WWProfileHierachyNodeClass -- Constructor                     *
 *   WWProfileHierachyNodeClass::~WWProfileHierachyNodeClass -- Destructor                     *
 *   WWProfileHierachyNodeClass::Get_Sub_Node -- Searches for a child node by name (pointer)   *
 *   WWProfileHierachyNodeClass::Reset -- Reset all profiling data in the tree                 *
 *   WWProfileHierachyNodeClass::Call -- Start timing                                          *
 *   WWProfileHierachyNodeClass::Return -- Stop timing, record results                         *
 *   WWProfileManager::Start_Profile -- Begin a named profile                                  *
 *   WWProfileManager::Stop_Profile -- Stop timing and record the results.                     *
 *   WWProfileManager::Reset -- Reset the contents of the profiling system                     *
 *   WWProfileManager::Increment_Frame_Counter -- Increment the frame counter                  *
 *   WWProfileManager::Get_Time_Since_Reset -- returns the elapsed time since last reset       *
 *   WWProfileManager::Get_Iterator -- Creates an iterator for the profile tree                *
 *   WWProfileManager::Release_Iterator -- Return an iterator for the profile tree             *
 *   WWProfileManager::Get_In_Order_Iterator -- Creates an "in-order" iterator for the profile *
 *   WWProfileManager::Release_In_Order_Iterator -- Return an "in-order" iterator              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "wwprofile.h"
#include "FastAllocator.h"
#include "wwdebug.h"
#include <windows.h>
//#include "systimer.h"
#include "systimer.h"
#include "rawfile.h"
#include "ffactory.h"
#include "simplevec.h"
#include "cpudetect.h"

static SimpleDynVecClass<WWProfileHierachyNodeClass*> ProfileCollectVector;
static double TotalFrameTimes;
static bool ProfileCollecting;

unsigned WWProfile_Get_System_Time()
{
	return TIMEGETTIME();
}

/***********************************************************************************************
 * WWProfile_Get_Ticks -- Retrieves the cpu performance counter                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
inline void WWProfile_Get_Ticks(_int64 * ticks)
{
#ifdef _UNIX
       *ticks = TIMEGETTIME();
#else
	__asm
	{
		push edx;
		push ecx;
		push eax;
		mov ecx,ticks;
		_emit 0Fh
		_emit 31h
		mov [ecx],eax;
		mov [ecx+4],edx;
		pop eax;
		pop ecx;
		pop edx;
	}
#endif
}


/***********************************************************************************************
 * WWProfileHierachyNodeClass::WWProfileHierachyNodeClass -- Constructor                       *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 * name - pointer to a static string which is the name of this profile node                    *
 * parent - parent pointer                                                                     *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * The name is assumed to be a static pointer, only the pointer is stored and compared for     *
 * efficiency reasons.                                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWProfileHierachyNodeClass::WWProfileHierachyNodeClass( const char * name, WWProfileHierachyNodeClass * parent ) :
	Name( name ),
	TotalCalls( 0 ),
	TotalTime( 0 ),
	StartTime( 0 ),
	RecursionCounter( 0 ),
	Parent( parent ),
	Child( NULL ),
	Sibling( NULL )
{
	Reset();
}


/***********************************************************************************************
 * WWProfileHierachyNodeClass::~WWProfileHierachyNodeClass -- Destructor                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWProfileHierachyNodeClass::~WWProfileHierachyNodeClass( void )
{
	delete Child;
	delete Sibling;
}


WWProfileHierachyNodeClass* WWProfileHierachyNodeClass::Clone_Hierarchy(WWProfileHierachyNodeClass* parent)
{
	WWProfileHierachyNodeClass* node=new WWProfileHierachyNodeClass(Name,parent);
	node->TotalCalls=TotalCalls;
	node->TotalTime=TotalTime;
	node->StartTime=StartTime;
	node->RecursionCounter=RecursionCounter;
	
	if (Child) {
		node->Child=Child->Clone_Hierarchy(this);
	}
	if (Sibling) {
		node->Sibling=Sibling->Clone_Hierarchy(parent);
	}

	return node;
}

void WWProfileHierachyNodeClass::Write_To_File(FileClass* file,int recursion)
{
	if (TotalTime!=0.0f) {
		int i;
		StringClass string;
		StringClass work;
		for (i=0;i<recursion;++i) { string+="\t"; }
		work.Format("%s\t%d\t%f\r\n",Name,TotalCalls,TotalTime*1000.0f);
		string+=work;
		file->Write(string.Peek_Buffer(),string.Get_Length());
	}
	if (Child) {
		Child->Write_To_File(file,recursion+1);
	}
	if (Sibling) {
		Sibling->Write_To_File(file,recursion);
	}
}


/***********************************************************************************************
 * WWProfileHierachyNodeClass::Get_Sub_Node -- Searches for a child node by name (pointer)     *
 *                                                                                             *
 * INPUT:                                                                                      *
 * name - static string pointer to the name of the node we are searching for                   *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * All profile names are assumed to be static strings so this function uses pointer compares   *
 * to find the named node.                                                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWProfileHierachyNodeClass * WWProfileHierachyNodeClass::Get_Sub_Node( const char * name )
{
	// Try to find this sub node
	WWProfileHierachyNodeClass * child = Child;
	while ( child ) {
		if ( child->Name == name ) {
			return child;
		}
		child = child->Sibling;
	}

	// We didn't find it, so add it
	WWProfileHierachyNodeClass * node = new WWProfileHierachyNodeClass( name, this );
	node->Sibling = Child;
	Child = node;
	return node;
}


/***********************************************************************************************
 * WWProfileHierachyNodeClass::Reset -- Reset all profiling data in the tree                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void	WWProfileHierachyNodeClass::Reset( void )
{
	TotalCalls = 0;
	TotalTime = 0.0f;

	if ( Child ) {
		Child->Reset();
	}
	if ( Sibling ) {
		Sibling->Reset();
	}
}


/***********************************************************************************************
 * WWProfileHierachyNodeClass::Call -- Start timing                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void	WWProfileHierachyNodeClass::Call( void )
{
	TotalCalls++;
	if (RecursionCounter++ == 0) {
		WWProfile_Get_Ticks(&StartTime);
	}
}


/***********************************************************************************************
 * WWProfileHierachyNodeClass::Return -- Stop timing, record results                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
bool	WWProfileHierachyNodeClass::Return( void )
{
	if (--RecursionCounter == 0) {
		if ( TotalCalls != 0 ) {
			int64_t time;
			WWProfile_Get_Ticks(&time);
			time-=StartTime;

			TotalTime += float(double(time)*CPUDetectClass::Get_Inv_Processor_Ticks_Per_Second());
		}
	}
	return RecursionCounter == 0;
}


/***************************************************************************************************
**
** WWProfileManager Implementation
**
***************************************************************************************************/
WWProfileHierachyNodeClass		WWProfileManager::Root( "Root", NULL );
WWProfileHierachyNodeClass	*	WWProfileManager::CurrentNode = &WWProfileManager::Root;
WWProfileHierachyNodeClass	*	WWProfileManager::CurrentRootNode = &WWProfileManager::Root;
int									WWProfileManager::FrameCounter = 0;
int64_t								WWProfileManager::ResetTime = 0;

static unsigned int				ThreadID = static_cast<unsigned int>(-1);


/***********************************************************************************************
 * WWProfileManager::Start_Profile -- Begin a named profile                                    *
 *                                                                                             *
 * Steps one level deeper into the tree, if a child already exists with the specified name     *
 * then it accumulates the profiling; otherwise a new child node is added to the profile tree. *
 *                                                                                             *
 * INPUT:                                                                                      *
 * name - name of this profiling record                                                        *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * The string used is assumed to be a static string; pointer compares are used throughout      *
 * the profiling code for efficiency.                                                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void	WWProfileManager::Start_Profile( const char * name )
{
	if (::GetCurrentThreadId() != ThreadID) {
		return;
	}

//	int current_thread = ::GetCurrentThreadId();
	if (name != CurrentNode->Get_Name()) {
		CurrentNode = CurrentNode->Get_Sub_Node( name );
	}

	CurrentNode->Call();
}

void	WWProfileManager::Start_Root_Profile( const char * name )
{
	if (::GetCurrentThreadId() != ThreadID) {
		return;
	}

	if (name != CurrentRootNode->Get_Name()) {
		CurrentRootNode = CurrentRootNode->Get_Sub_Node( name );
	}

	CurrentRootNode->Call();
}


/***********************************************************************************************
 * WWProfileManager::Stop_Profile -- Stop timing and record the results.                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void	WWProfileManager::Stop_Profile( void )
{
	if (::GetCurrentThreadId() != ThreadID) {
		return;
	}

	// Return will indicate whether we should back up to our parent (we may
	// be profiling a recursive function)
	if (CurrentNode->Return()) {
		CurrentNode = CurrentNode->Get_Parent();
	}
}

void	WWProfileManager::Stop_Root_Profile( void )
{
	if (::GetCurrentThreadId() != ThreadID) {
		return;
	}

	// Return will indicate whether we should back up to our parent (we may
	// be profiling a recursive function)
	if (CurrentRootNode->Return()) {
		CurrentRootNode = CurrentRootNode->Get_Parent();
	}
}


/***********************************************************************************************
 * WWProfileManager::Reset -- Reset the contents of the profiling system                       *
 *                                                                                             *
 *    This resets everything except for the tree structure.  All of the timing data is reset.  *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void	WWProfileManager::Reset( void )
{  
	ThreadID = ::GetCurrentThreadId();

	Root.Reset();
	FrameCounter = 0;
	WWProfile_Get_Ticks(&ResetTime);
}


/***********************************************************************************************
 * WWProfileManager::Increment_Frame_Counter -- Increment the frame counter                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void WWProfileManager::Increment_Frame_Counter( void )
{
	if (ProfileCollecting) {
		float time=Get_Time_Since_Reset();
		TotalFrameTimes+=time;
		WWProfileHierachyNodeClass* new_root=Root.Clone_Hierarchy(NULL);
		new_root->Set_Total_Time(time);
		new_root->Set_Total_Calls(1);
		ProfileCollectVector.Add(new_root);
		Reset();
	}

	FrameCounter++;

}


/***********************************************************************************************
 * WWProfileManager::Get_Time_Since_Reset -- returns the elapsed time since last reset         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
float WWProfileManager::Get_Time_Since_Reset( void )
{
	int64_t time;
	WWProfile_Get_Ticks(&time);
	time -= ResetTime;

	return float(double(time) * CPUDetectClass::Get_Inv_Processor_Ticks_Per_Second());
}


/***********************************************************************************************
 * WWProfileManager::Get_Iterator -- Creates an iterator for the profile tree                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWProfileIterator *	WWProfileManager::Get_Iterator( void )
{
	return new WWProfileIterator( &Root );
}


/***********************************************************************************************
 * WWProfileManager::Release_Iterator -- Return an iterator for the profile tree               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void	WWProfileManager::Release_Iterator( WWProfileIterator * iterator )
{
	delete iterator;
}


void	WWProfileManager::Begin_Collecting()
{
	Reset();
	ProfileCollecting=true;
	TotalFrameTimes=0.0;
}

void	WWProfileManager::End_Collecting(const char* filename)
{
	int i;
	if (filename && ProfileCollectVector.Count()!=0) {
		FileClass * file= _TheWritingFileFactory->Get_File(filename);	
		if (file != NULL) {
			//
			//	Open or create the file
			//
			file->Open (FileClass::WRITE);

			StringClass str;
			float avg_frame_time=TotalFrameTimes/float(ProfileCollectVector.Count());
			str.Format(
				"Total frames: %d, average frame time: %fms\r\n"
				"All frames taking more than twice the average frame time are marked with keyword SPIKE.\r\n\r\n",
				ProfileCollectVector.Count(),avg_frame_time*1000.0f);
			file->Write(str.Peek_Buffer(),str.Get_Length());

			for (i=0;i<ProfileCollectVector.Count();++i) {
				float frame_time=ProfileCollectVector[i]->Get_Total_Time();
				str.Format("FRAME: %d %fms %s ---------------\r\n",i,frame_time*1000.0f,frame_time>avg_frame_time*2.0f ? "SPIKE" : "");
				file->Write(str.Peek_Buffer(),str.Get_Length());
				ProfileCollectVector[i]->Write_To_File(file,0);
			}
		
			//
			//	Close the file
			//
			file->Close ();
			_TheWritingFileFactory->Return_File (file);
		}
	}

	for (i=0;i<ProfileCollectVector.Count();++i) {
		delete ProfileCollectVector[i];
		ProfileCollectVector[i]=0;
	}
	ProfileCollectVector.Delete_All();
	ProfileCollecting=false;
}



/***********************************************************************************************
 * WWProfileManager::Get_In_Order_Iterator -- Creates an "in-order" iterator for the profile t *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWProfileInOrderIterator * WWProfileManager::Get_In_Order_Iterator( void )
{
	return new WWProfileInOrderIterator;
}


/***********************************************************************************************
 * WWProfileManager::Release_In_Order_Iterator -- Return an "in-order" iterator                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/24/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void	WWProfileManager::Release_In_Order_Iterator( WWProfileInOrderIterator * iterator )
{
	delete iterator;
}


/***************************************************************************************************
**
** WWProfileIterator Implementation
**
***************************************************************************************************/
WWProfileIterator::WWProfileIterator( WWProfileHierachyNodeClass * start )
{
	CurrentParent = start;
	CurrentChild = CurrentParent->Get_Child();
}

void	WWProfileIterator::First(void)
{
	CurrentChild = CurrentParent->Get_Child();
}


void	WWProfileIterator::Next(void)
{
	CurrentChild = CurrentChild->Get_Sibling();
}

bool	WWProfileIterator::Is_Done(void)
{
	return CurrentChild == NULL;
}

void	WWProfileIterator::Enter_Child( void )
{
	CurrentParent = CurrentChild;
	CurrentChild = CurrentParent->Get_Child();
}

void	WWProfileIterator::Enter_Child( int index )
{
	CurrentChild = CurrentParent->Get_Child();
	while ( (CurrentChild != NULL) && (index != 0) ) {
		index--;
		CurrentChild = CurrentChild->Get_Sibling();
	}

	if ( CurrentChild != NULL ) {
		CurrentParent = CurrentChild;
		CurrentChild = CurrentParent->Get_Child();
	}
}

void	WWProfileIterator::Enter_Parent( void )
{
	if ( CurrentParent->Get_Parent() != NULL ) {
		CurrentParent = CurrentParent->Get_Parent();
	}
	CurrentChild = CurrentParent->Get_Child();
}

/***************************************************************************************************
**
** WWProfileInOrderIterator Implementation
**
***************************************************************************************************/

WWProfileInOrderIterator::WWProfileInOrderIterator( void )
{
	CurrentNode = &WWProfileManager::Root;
}

void	WWProfileInOrderIterator::First(void)
{
	CurrentNode = &WWProfileManager::Root;
}

void	WWProfileInOrderIterator::Next(void)
{
	if ( CurrentNode->Get_Child() ) {				// If I have a child, go to child
		CurrentNode = CurrentNode->Get_Child();
	} else if ( CurrentNode->Get_Sibling() ) {	// If I have a sibling, go to sibling
		CurrentNode = CurrentNode->Get_Sibling();
	} else {											//	if not, go to my parent's sibling, or his.......
		// Find a parent with a sibling....
		bool done = false;
		while ( CurrentNode != NULL && !done ) {

			// go to my parent
			CurrentNode = CurrentNode->Get_Parent();

			// If I have a sibling, go there
			if ( CurrentNode != NULL && CurrentNode->Get_Sibling() != NULL ) {
				CurrentNode = CurrentNode->Get_Sibling();
				done = true;
			}
		}
	}
}

bool	WWProfileInOrderIterator::Is_Done(void)
{
	return CurrentNode == NULL;
}

/*
**
*/
WWTimeItClass::WWTimeItClass( const char * name )
{
	Name = name;
	WWProfile_Get_Ticks( &Time );
}

WWTimeItClass::~WWTimeItClass( void )
{
	int64_t End;
	WWProfile_Get_Ticks( &End );
	End -= Time;
#ifdef WWDEBUG
	float time = End * CPUDetectClass::Get_Inv_Processor_Ticks_Per_Second();
	WWDEBUG_SAY(( "*** WWTIMEIT *** %s took %1.9f\n", Name, time ));
#endif
}


/*
**
*/
WWMeasureItClass::WWMeasureItClass( float * p_result )
{
	WWASSERT(p_result != NULL);
	PResult = p_result;
	WWProfile_Get_Ticks( &Time );
}

WWMeasureItClass::~WWMeasureItClass( void )
{
	int64_t End;
	WWProfile_Get_Ticks( &End );
	End -= Time;
	WWASSERT(PResult != NULL);
	*PResult = End  * CPUDetectClass::Get_Inv_Processor_Ticks_Per_Second();
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

unsigned WWMemoryAndTimeLog::TabCount;

WWMemoryAndTimeLog::WWMemoryAndTimeLog(const char* name)
	:
	Name(name),
	TimeStart(WWProfile_Get_System_Time()),
	AllocCountStart(FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocation_Count()),
	AllocSizeStart(FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocated_Size())
{
	IntermediateTimeStart=TimeStart;
	IntermediateAllocCountStart=AllocCountStart;
	IntermediateAllocSizeStart=AllocSizeStart;
	StringClass tmp(0,true);
	for (unsigned i=0;i<TabCount;++i) tmp+="\t";
	WWRELEASE_SAY(("%s%s {\n",tmp,name));
	TabCount++;
}

WWMemoryAndTimeLog::~WWMemoryAndTimeLog()
{
	if (TabCount>0) TabCount--;
	StringClass tmp(0,true);
	for (unsigned i=0;i<TabCount;++i) tmp+="\t";
	WWRELEASE_SAY(("%s} ",tmp));

	unsigned current_time=WWProfile_Get_System_Time();
	int current_alloc_count=FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocation_Count();
	int current_alloc_size=FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocated_Size();
	WWRELEASE_SAY(("IN TOTAL %s took %d.%3.3d s, did %d memory allocations of %d bytes\n",
		Name,
		(current_time - TimeStart)/1000, (current_time - TimeStart)%1000,
		current_alloc_count - AllocCountStart,
		current_alloc_size - AllocSizeStart));
	WWRELEASE_SAY(("\n"));

}


void WWMemoryAndTimeLog::Log_Intermediate(const char* text)
{
	unsigned current_time=WWProfile_Get_System_Time();
	int current_alloc_count=FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocation_Count();
	int current_alloc_size=FastAllocatorGeneral::Get_Allocator()->Get_Total_Allocated_Size();
	StringClass tmp(0,true);
	for (unsigned i=0;i<TabCount;++i) tmp+="\t";
	WWRELEASE_SAY(("%s%s took %d.%3.3d s, did %d memory allocations of %d bytes\n",
		tmp,
		text,
		(current_time - IntermediateTimeStart)/1000, (current_time - IntermediateTimeStart)%1000,
		current_alloc_count - IntermediateAllocCountStart,
		current_alloc_size - IntermediateAllocSizeStart));
	IntermediateTimeStart=current_time;
	IntermediateAllocCountStart=current_alloc_count;
	IntermediateAllocSizeStart=current_alloc_size;
}