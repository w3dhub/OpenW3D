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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/vistablemgr.cpp                       $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 4/20/01 10:00a                                              $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "vistablemgr.h"
#include "vistable.h"
#include "chunkio.h"
#include "wwmemlog.h"


const int VIS_LRU_FRAMES = 5;

/**
** VisDecompressionCacheClass
** This class encapsulates an LRU cache for decompressed vis tables.  This was needed because
** light-sources now use VIS data and didn't want to pay the cpu cost of decompressing their
** vis table each time it is needed or the ram cost of keeping them all decompressed all of the
** time.
*/
class VisDecompressionCacheClass
{
public:
	VisDecompressionCacheClass(void) { }
	~VisDecompressionCacheClass(void) { Reset(0); }

	void						Reset(int vis_sector_count = -1);

	VisTableClass *		Get_Table(int vis_sector_id);
	void						Add_Table(VisTableClass * pvs);
	void						Release_Old_Tables(void);

	void						Set_Current_Timestamp(int timestamp)		{ CurrentTimestamp = timestamp; }
	int						Get_Current_Timestamp(void)					{ return CurrentTimestamp; }

protected:

	SimpleVecClass<VisTableClass *>			Cache;
	MultiListClass<VisTableClass>				LRUQueue;

	int												CurrentTimestamp;
};


void VisDecompressionCacheClass::Reset(int vis_sector_count)
{
	/*
	** Each table that we have should be in our LRU list
	*/
	VisTableClass * pvs = LRUQueue.Peek_Head();
	while (pvs != NULL) {
		WWASSERT(Cache[pvs->Get_Vis_Sector_ID()] != NULL);

		LRUQueue.Remove_Head();

		int vis_id = pvs->Get_Vis_Sector_ID();
		REF_PTR_RELEASE(Cache[vis_id]);

		pvs = LRUQueue.Peek_Head();
	}

	/*
	** Sanity check, every pointer in the cache array should now be NULL!
	*/
#ifdef WWDEBUG
	for (int i=0; i<Cache.Length(); i++) {
		WWASSERT(Cache[i] == NULL);
	}
#endif

	/*
	** Resize the pointer array to match the vis sector count if needed
	*/
	if (vis_sector_count != -1) {
		Cache.Resize(vis_sector_count);
		Cache.Zero_Memory();
	}
}


VisTableClass *
VisDecompressionCacheClass::Get_Table(int vis_sector_id)
{
	if (Cache[vis_sector_id] != NULL) {
		/*
		** Move the table to the end of the LRU Queue and
		** update its timestamp
		*/
		LRUQueue.Remove(Cache[vis_sector_id]);
		Cache[vis_sector_id]->Set_Time_Stamp(CurrentTimestamp);
		LRUQueue.Add_Tail(Cache[vis_sector_id]);

		/*
		** Add a ref for the user and return the table
		*/
		Cache[vis_sector_id]->Add_Ref();
		return Cache[vis_sector_id];
	} else {
		return NULL;
	}
}

void
VisDecompressionCacheClass::Add_Table(VisTableClass * pvs)
{
	WWASSERT(Cache[pvs->Get_Vis_Sector_ID()] == NULL);
	WWASSERT(pvs != NULL);

	/*
	** Set the timestamp, add to the cache, and add to
	** the tail of the LRUQueue
	*/
	pvs->Set_Time_Stamp(CurrentTimestamp);
	REF_PTR_SET(Cache[pvs->Get_Vis_Sector_ID()],pvs);
	LRUQueue.Add_Tail(pvs);
}

void
VisDecompressionCacheClass::Release_Old_Tables(void)
{
	/*
	** Release any vis table that hasn't been used in X frames
	*/
	int timestamp_cutoff = CurrentTimestamp - VIS_LRU_FRAMES;

	VisTableClass * tbl = LRUQueue.Peek_Head();
	while (tbl && tbl->Get_Time_Stamp() < timestamp_cutoff) {

		int vis_id = tbl->Get_Vis_Sector_ID();
		REF_PTR_RELEASE(Cache[vis_id]);

		LRUQueue.Remove_Head();
		tbl = LRUQueue.Peek_Head();
	}
}


/*******************************************************************************************
**
** VisTableManagerClass Implemenation
**
*******************************************************************************************/


/*
** Save/Load constants
*/
#define VISMGR_CURRENT_VERSION		0x00010001

enum
{
	VISMGR_CHUNK_VARIABLES				= 0x34500000,
	VISMGR_CHUNK_VISTABLEID				= 0x34500001,
	VISMGR_CHUNK_VISTABLEDATA			= 0x34500002,

	VISMGR_VARIABLE_VERSION				= 0x00,
	VISMGR_VARIABLE_IDCOUNT,								// OBSOLETE
	VISMGR_VARIABLE_VISOBJECTCOUNT,
	VISMGR_VARIABLE_VISSECTORCOUNT,
};




VisTableMgrClass::VisTableMgrClass(void) :
	VisSectorCount(0),
	VisObjectCount(0),
	FrameCounter(0)
{
	WWMEMLOG(MEM_VIS);
	Cache = new VisDecompressionCacheClass;
	Reset();
}

VisTableMgrClass::~VisTableMgrClass(void)
{
	Reset ();
	delete Cache;
}

void VisTableMgrClass::Reset(void)
{
	/*
	** delete all allocated tables
	*/
	Delete_All_Vis_Tables();

	/*
	** reset the vector of pointers
	*/
	VisTables.Delete_All();

	/*
	** reset the id counters
	*/
	VisObjectCount = 1;			// always reserve Object ID 0 for objects that are forced visibile
	VisSectorCount = 0;
	FrameCounter = 0;

	/*
	** reset the decompression cache
	*/
	Cache->Reset(0);
}

int VisTableMgrClass::Allocate_Vis_Object_ID(int count /*= 1*/)
{
	int id = VisObjectCount;
	VisObjectCount += count;
	return id;
}

void	VisTableMgrClass::Set_Optimized_Vis_Object_Count(int count)
{
	/*
	** After the optimization process, we need to simply force the VisObjectCount
	*/
	VisObjectCount = count;
}

int VisTableMgrClass::Allocate_Vis_Sector_ID(int count /*= 1*/)
{
	int id = VisSectorCount;

	for (int i=0; i<count; i++) {
		VisTables.Add(NULL);
	}

	VisSectorCount += count;
	WWASSERT(VisSectorCount == VisTables.Count());

	Cache->Reset(VisSectorCount);
	return id;
}

int VisTableMgrClass::Get_Vis_Table_Size(void) const
{
	return VisObjectCount;
}

int VisTableMgrClass::Get_Vis_Table_Count(void) const
{
	return VisSectorCount;
}

VisTableClass * VisTableMgrClass::Get_Vis_Table(int id,bool allocate/*= false*/)
{
	WWMEMLOG(MEM_VIS);

	/*
	** if id is invalid, return NULL
	*/
	if ((id == -1) || (id >= VisTables.Count())) {
#ifdef WWDEBUG
		if (id > 0) WWDEBUG_SAY(("Vis Table ID out of range: %d\r\n",id));
#endif
		return NULL;
	}

	/*
	** - if the cache has the decompressed table just return a pointer
	** - if we have the compressed version of this vis table, decompress it and return it
	** - else if they want one allocated, just create a table and return it
	**   (we will install the compressed version when they release this table)
	** - else return NULL
	*/
	VisTableClass * pvs = Cache->Get_Table(id);

	if (pvs != NULL) {

		/*
		** Cache had the table, just return the pointer. (Add-Ref'd by the cache...)
		*/
		return pvs;

	} else if (VisTables[id] != NULL) {

		/*
		** Cache didn't have it, but we have the compressed version.
		** Decompress, add to the cache, and return the table.
		*/
		pvs = NEW_REF(VisTableClass,(VisTables[id],Get_Vis_Table_Size(),id));
		Cache->Add_Table(pvs);
		return pvs;

	} else if (allocate) {

		/*
		** This table doesn't exist yet but the user wants to allocate
		** it.  Create a new vis table for him and return it
		*/
		VisTableClass * new_table = NEW_REF(VisTableClass,(Get_Vis_Table_Size(),id));
		new_table->Set_Bit(0,true); // Always set ID 0 to be visible...
		return new_table;

	} else {

		/*
		** Table doesn't exist, just return NULL
		*/
		return NULL;

	}
}

void VisTableMgrClass::Update_Vis_Table(int id,VisTableClass * pvs)
{
	WWMEMLOG(MEM_VIS);
	WWASSERT(pvs->Get_Bit_Count() == VisObjectCount);

	/*
	** release any existing vis table
	*/
	if (VisTables[id] != NULL) {
		delete VisTables[id];
		VisTables[id] = NULL;
	}

	/*
	** compress this one
	*/
	VisTables[id] = new CompressedVisTableClass(pvs);

	/*
	** Reset the decompression cache
	*/
	Cache->Reset();
}

bool VisTableMgrClass::Has_Vis_Table(int id)
{
	return (VisTables[id] != NULL);
}

void VisTableMgrClass::Notify_Frame_Ended(void)
{
	FrameCounter++;
	Cache->Set_Current_Timestamp(FrameCounter);
	Cache->Release_Old_Tables();
}

void VisTableMgrClass::Delete_All_Vis_Tables(void)
{
	// delete all allocated tables
	for (int i=0; i<VisTables.Count(); i++) {
		if (VisTables[i] != NULL) {
			delete VisTables[i];
			VisTables[i] = NULL;
		}
	}
}


void VisTableMgrClass::Save(ChunkSaveClass & csave)
{
	WWMEMLOG(MEM_VIS);

	uint32 version = VISMGR_CURRENT_VERSION;

	csave.Begin_Chunk(VISMGR_CHUNK_VARIABLES);
	WRITE_MICRO_CHUNK(csave,VISMGR_VARIABLE_VERSION,version);
	WRITE_MICRO_CHUNK(csave,VISMGR_VARIABLE_VISOBJECTCOUNT,VisObjectCount);
	WRITE_MICRO_CHUNK(csave,VISMGR_VARIABLE_VISSECTORCOUNT,VisSectorCount);
	csave.End_Chunk();

	// loop over tables
	for (int i=0; i<VisTables.Count(); i++) {

		if (VisTables[i] != NULL) {
			csave.Begin_Chunk(VISMGR_CHUNK_VISTABLEID);
			uint32 index = i;
			csave.Write(&index,sizeof(index));
			csave.End_Chunk();

			csave.Begin_Chunk(VISMGR_CHUNK_VISTABLEDATA);
			VisTables[i]->Save(csave);
			csave.End_Chunk();
		}
	}
}


void VisTableMgrClass::Load(ChunkLoadClass & cload)
{
	WWMEMLOG(MEM_VIS);

	// read in the variables
	uint32 version = 0;
	cload.Open_Chunk();
	WWASSERT(cload.Cur_Chunk_ID() == VISMGR_CHUNK_VARIABLES);
	while (cload.Open_Micro_Chunk()) {
		switch(cload.Cur_Micro_Chunk_ID()) {
			READ_MICRO_CHUNK(cload,VISMGR_VARIABLE_VERSION,version);
			READ_MICRO_CHUNK(cload,VISMGR_VARIABLE_VISOBJECTCOUNT,VisObjectCount);
			READ_MICRO_CHUNK(cload,VISMGR_VARIABLE_VISSECTORCOUNT,VisSectorCount);
		}
		cload.Close_Micro_Chunk();
	}
	cload.Close_Chunk();

	if (version < 0x00010001) {
		WWDEBUG_SAY(("Obsolete Vis-Data detected, Reseting Visiblity System!\r\n"));
		Reset();
		return;
	}

	// allocate a pointer for each table
	for (int i=0; i<VisSectorCount; i++) {
		VisTables.Add(NULL,VisSectorCount);
	}

	// reset the cache
	Cache->Reset(VisSectorCount);

	// read the actual vis tables
	uint32 id = 0xFFFFFFFF;

	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) {

			case VISMGR_CHUNK_VISTABLEID:
				cload.Read(&id,sizeof(id));
				break;

			case VISMGR_CHUNK_VISTABLEDATA:
				WWASSERT(id != 0xFFFFFFFF);
				if (VisTables[id] == NULL) {
					VisTables[id] = new CompressedVisTableClass;
				}
				VisTables[id]->Load(cload);

				id = 0xFFFFFFFF;
				break;
		}
		cload.Close_Chunk();
	}
}



