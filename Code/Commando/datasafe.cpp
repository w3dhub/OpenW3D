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
 *                     $Archive:: /Commando/Code/Commando/datasafe.cpp                        $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/10/02 2:38p                                              $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "always.h"
#include "crandom.h"
#include "datasafe.h"
#include "vector.h"

#include <malloc.h>
#include <windows.h>
#include "systimer.h"
#include <stdio.h>

/*
** Renegade specific includes. For reporting tampering.
*/
#include "cstextobj.h"
#include "cnetwork.h"
#include "player.h"
#include "playermanager.h"
#include "devoptions.h"
#include "string_ids.h"
#include "translatedb.h"
#include <algorithm>

/*
** Put this data in the lib init area so it gets initialised first.
*/
//#pragma init_seg(lib)

/*
** Static data. See class definition for details.
**
** I need to make this stuff static so that I can templatize the derived class and have all expansions use the same data.
*/
unsigned int GenericDataSafeClass::SimpleKey;
unsigned int GenericDataSafeClass::HandleKey;
unsigned int GenericDataSafeClass::Checksum;
unsigned int GenericDataSafeClass::ShuffleDelay;
unsigned int GenericDataSafeClass::SecurityCheckDelay;
DataSafeHandleClass GenericDataSafeClass::SentinelOne = 0;
int GenericDataSafeClass::NumLists = 0;
DataSafeEntryListClass *GenericDataSafeClass::Safe[MAX_DATASAFE_LISTS];
DataSafeEntryTypeClass GenericDataSafeClass::TypeList[MAX_DATASAFE_TYPES];
int GenericDataSafeClass::TypeListCount = 0;
DataSafeHandleClass GenericDataSafeClass::SentinelTwo = 0;
int GenericDataSafeClass::CRCErrors = 0;
#ifdef THREAD_SAFE_DATA_SAFE
HANDLE GenericDataSafeClass::SafeMutex;
#else //THREAD_SAFE_DATA_SAFE
unsigned int GenericDataSafeClass::PreferredThread = GetCurrentThreadId();
#endif //THREAD_SAFE_DATA_SAFE

#ifdef WWDEBUG
unsigned int GenericDataSafeClass::LastDump = TIMEGETTIME();
int GenericDataSafeClass::NumSwaps = 0;
int GenericDataSafeClass::NumFetches = 0;
int GenericDataSafeClass::SlopCount = 0;
int GenericDataSafeClass::NumSecurityChecks = 0;
int GenericDataSafeClass::NumShuffles = 0;
#endif	//WWDEBUG

static DynamicVectorClass<int> random_list_contenders;

/*
** Buffer to return a reference to in error cases.
*/
char ErrorVal[1024] = {0,0,0,0};

/*
** Instances of data safes. One for each type of data we will be storing in the safe.
**
** For each type, we have to declare more than just the class, we have to declare static data too. This is taken care of by
** the DECLARE_DATA_SAFE macro.
**
** All data safe type MUST be declared here so that they are constructed after the ::TypeList and ::TypeListCount statics above.
**
** A data safe of type 'int' must always be declared first.
**
*/
typedef unsigned int DATASAFE_UNSIGNED_INT;
DECLARE_DATA_SAFE(int);
DECLARE_DATA_SAFE(DATASAFE_UNSIGNED_INT);
DECLARE_DATA_SAFE(float);
DECLARE_DATA_SAFE(double);

/*
** Define this to set the keys to a fixed value to aid debugging.
*/
//#define FIXED_KEY


/***********************************************************************************************
 * GenericDataSafeClass::GenericDataSafeClass -- Class constructor.                            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 2:58PM ST : Created                                                              *
 *=============================================================================================*/
GenericDataSafeClass::GenericDataSafeClass(void)
{
	/*
	** This constructor could be called lots of times as new types are added to the data safe. Make sure we don't get an
	** unexpected reset if we have already added types.
	*/
	if (TypeListCount == 0) {

#ifdef THREAD_SAFE_DATA_SAFE
		SafeMutex = CreateMutexA(NULL, false, NULL);
#else
		PreferredThread = GetCurrentThreadId();
#endif //THREAD_SAFE_DATA_SAFE

#ifdef FIXED_KEY
		SimpleKey = 0x80000000;
		HandleKey = 0x40000000;
#else	//FIXED_KEY
		SimpleKey = 0x55555555 ^ TIMEGETTIME() ^ FreeRandom.Get_Int(0, 0xffffffff);
		HandleKey = 0xaaaaaaaa ^ (TIMEGETTIME()*2) ^ FreeRandom.Get_Int(0, 0xffffffff);
#endif	//FIXED_KEY
		NumLists = 0;
		Checksum = ~SimpleKey;
		ShuffleDelay = TIMEGETTIME();
		SecurityCheckDelay = TIMEGETTIME();
		memset(&Safe[0], 0, sizeof(Safe));

		Reset();

	} else {
		if (TypeListCount == 1 && SentinelOne == 0 && SentinelTwo == 0) {

			/*
			** Add sentinels if we already know about the 'int' type.
			*/
			int s1 = SENTINEL_ONE;
			int s2 = SENTINEL_TWO;
			SentinelOne = DataSafeClass<int>::Add_Entry(s1);
			SentinelTwo = DataSafeClass<int>::Add_Entry(s2);
		}
	}
}



/***********************************************************************************************
 * GenericDataSafeClass::Reset_Timers -- Handle timer reset                                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/31/2002 12:12PM ST : Created                                                            *
 *=============================================================================================*/
void GenericDataSafeClass::Reset_Timers(void)
{
	ShuffleDelay = TIMEGETTIME();
	SecurityCheckDelay = TIMEGETTIME();
#ifdef WWDEBUG
	LastDump = TIMEGETTIME();
#endif //WWDEBUG
}



/***********************************************************************************************
 * GenericDataSafeClass::Reset -- Reset the safe stats.                                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/30/2001 11:38AM ST : Created                                                            *
 *=============================================================================================*/
void GenericDataSafeClass::Reset(void)
{
	CRCErrors = 0;

#ifdef WWDEBUG
	LastDump = TIMEGETTIME();
	NumSwaps = 0;
	NumFetches = 0;
	SlopCount = 0;
	NumSecurityChecks = 0;
	NumShuffles = 0;
#endif	//WWDEBUG

	if (NumLists) {
		Shuffle();
		Security_Check();
	}
}



/***********************************************************************************************
 * GenericDataSafeClass::Shutdown -- Free any remaining memory if we are finished with the safe*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/6/2001 1:47PM ST : Created                                                              *
 *=============================================================================================*/
void GenericDataSafeClass::Shutdown(void)
{
	/*
	** If there is nothing left in the data safe - not even slop - then free the list memory.
	*/
	bool found = false;
	for (int i=0 ; i<NumLists ; i++) {
		ds_assert(Safe[i] != NULL);

		if (Safe[i] != NULL && Safe[i]->EntryCount != 0) {
			found = true;
			break;
		}
	}

	if (!found) {
		for (int i=0 ; i<NumLists ; i++) {
			delete Safe[i];
		}
		NumLists = 0;

#ifdef THREAD_SAFE_DATA_SAFE
		CloseHandle(SafeMutex);
#endif //THREAD_SAFE_DATA_SAFE
	}
}




/***********************************************************************************************
 * GenericDataSafeClass::Get_Entry -- Find a raw entry in the data safe                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Handle                                                                            *
 *                                                                                             *
 * OUTPUT:   Pointer to raw entry (including header) or NULL if not found                      *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/13/2001 9:43PM ST : Created                                                             *
 *=============================================================================================*/
DataSafeEntryClass *GenericDataSafeClass::Get_Entry(DataSafeHandleClass handle)
{
	int list;

#ifdef	WWDEBUG
	NumFetches++;
#endif	//WWDEBUG

	/*
	** Check safe integrity.
	*/
	Security_Check();

	/*
	** The handles we give out are encrypted so we need to decrypt temporarily to extract the list number.
	*/
	DataSafeHandleClass new_handle = handle ^ HandleKey;
	list = new_handle.Handle.Part.List;

	/*
	** Apply the current data key to the handle for matching purposes.
	*/
	DataSafeHandleClass match_handle = new_handle ^ SimpleKey;

	ds_assert(list >= 0);
	ds_assert(list < NumLists);
	ds_assert(Safe[list] != NULL);

	/*
	** Get the head of the list.
	*/
	DataSafeEntryClass *entry_ptr = Safe[list]->SafeList;

	/*
	** Search through until we find a matching handle. Check to make sure we don't get stuck in an infinite loop.
	*/
	int times = 0;
	while (entry_ptr) {
		if (entry_ptr->Handle == match_handle) {
			return(entry_ptr);
		}
		entry_ptr = entry_ptr->Next;
		times++;
		ds_assert(times <= MAX_ENTRIES_PER_LIST);
		if (times > MAX_ENTRIES_PER_LIST) {
			entry_ptr = NULL;
			break;
		}
	}

	/*
	** Falling through to here means no match found and we return NULL as an error.
	*/
	WWDEBUG_SAY(("WARNING: Data Safe: No item found for handle %08x\n", (int)handle));
	ds_assert(entry_ptr != NULL);
	return(NULL);
}




/***********************************************************************************************
 * GenericDataSafeClass::Get_Entry -- Find a raw entry in the data safe                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Handle                                                                            *
 *                                                                                             *
 * OUTPUT:   Type of object that handle represents                                             *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/17/2001 12:44PM ST : Created                                                            *
 *=============================================================================================*/
int GenericDataSafeClass::Get_Entry_Type(DataSafeHandleClass handle)
{
	int list;

	/*
	** Check safe integrity.
	*/
	Security_Check();

	/*
	** The handles we give out are encrypted so we need to decrypt temporarily to extract the list number.
	*/
	DataSafeHandleClass new_handle = handle ^ HandleKey;
	list = new_handle.Handle.Part.List;

	ds_assert(list >= 0);
	ds_assert(list < NumLists);
	ds_assert(Safe[list] != NULL);

	/*
	** Return the type of data stored in this list.
	*/
	return(Safe[list]->EntryType);
}


/***********************************************************************************************
 * GenericDataSafeClass::Get_Entry_By_Index -- Find the nth entry in the given data safe list  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    List number                                                                       *
 *           Entry number                                                                      *
 *                                                                                             *
 * OUTPUT:   Pointer to entry or NULL if not found                                             *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/13/2001 9:43PM ST : Created                                                             *
 *=============================================================================================*/
DataSafeEntryClass *GenericDataSafeClass::Get_Entry_By_Index(int list, int index)
{
#ifdef	WWDEBUG
	NumFetches++;
#endif	//WWDEBUG

	/*
	** Locals.
	*/
	int count = 0;

	/*
	** Input ds_asserts.
	*/
	ds_assert(list >= 0);
	ds_assert(list < NumLists);
	ds_assert(Safe[list] != NULL);

	/*
	** Get the head of the list.
	*/
	DataSafeEntryClass *entry_ptr = Safe[list]->SafeList;

	/*
	** Iterate through the list, counting entries, until we find the requested entry number.
	*/
	while (entry_ptr) {
		if (count == index) {
			return(entry_ptr);
		}
		entry_ptr = entry_ptr->Next;
		count++;
		ds_assert(count < MAX_ENTRIES_PER_LIST);
	}

	/*
	** Falling through to here means no match found and we return NULL as an error.
	*/
	WWDEBUG_SAY(("WARNING: Data Safe: No item found for list %d, index %d\n", list, index));
	ds_assert(entry_ptr != NULL);
	return(NULL);
}






/***********************************************************************************************
 * GenericDataSafeClass::Mem_Copy_Encrypt -- Copy and encrypt data in one pass.                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to data dest                                                                  *
 *           Ptr to data source                                                                *
 *           Length of data in bytes                                                           *
 *           Should we update the checksum                                                     *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/19/2001 9:29PM ST : Created                                                             *
 *=============================================================================================*/
void GenericDataSafeClass::Mem_Copy_Encrypt(void *dest, void *src, int size, bool do_checksum)
{
	ds_assert((size % 4) == 0);
	unsigned int temp;
	unsigned int *s = (unsigned int *) src;
	unsigned int *d = (unsigned int *) dest;

	if (do_checksum) {
		for (int i = 0 ; i < (size / 4) ; i++) {
			temp = *s++;
			temp = temp ^ SimpleKey;
			Checksum = Checksum ^ temp;
			*d++ = temp;
		}
	} else {
		for (int i = 0 ; i < (size / 4) ; i++) {
			*d++ = *s++ ^ SimpleKey;
		}
	}
}




/***********************************************************************************************
 * GenericDataSafeClass::Mem_Copy_Decrypt -- Copy and decrypt data in one pass.                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to data dest                                                                  *
 *           Ptr to data source                                                                *
 *           Length of data in bytes                                                           *
 *           Should we update the checksum                                                     *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/19/2001 9:29PM ST : Created                                                             *
 *=============================================================================================*/
void GenericDataSafeClass::Mem_Copy_Decrypt(void *dest, void *src, int size, bool do_checksum)
{
	ds_assert((size % 4) == 0);
	unsigned int temp;
	unsigned int *s = (unsigned int *) src;
	unsigned int *d = (unsigned int *) dest;

	if (do_checksum) {
		for (int i = 0 ; i < (size / 4) ; i++) {
			temp = *s++;
			Checksum = Checksum ^ temp;
			temp = temp ^ SimpleKey;
			*d++ = temp;
		}
	} else {
		for (int i = 0 ; i < (size / 4) ; i++) {
			*d++ = *s++ ^ SimpleKey;
		}
	}
}


/***********************************************************************************************
 * GenericDataSafeClass::Encrypt -- Encrypt data.                                              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to data to encrypt                                                            *
 *           Length of data in bytes                                                           *
 *           Key to use                                                                        *
 *           Should we update the checksum                                                     *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/19/2001 9:29PM ST : Created                                                             *
 *=============================================================================================*/
void GenericDataSafeClass::Encrypt(void *data, int size, unsigned int key, bool do_checksum)
{
	ds_assert((size % 4) == 0);
	unsigned int *data_ptr = (unsigned int*)data;

	if (do_checksum) {
		for (int i = 0 ; i < (size / 4) ; i++) {
			*data_ptr ^= key;
			Checksum ^= *data_ptr++;
		}
	} else {
		for (int i = 0 ; i < (size / 4) ; i++) {
			*data_ptr ^= key;
		}
	}
}


/***********************************************************************************************
 * GenericDataSafeClass::Decrypt -- Decrypt data.                                              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to data to decrypt                                                            *
 *           Length of data in bytes                                                           *
 *           Key to use                                                                        *
 *           Should we update the checksum                                                     *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/19/2001 9:29PM ST : Created                                                             *
 *=============================================================================================*/
void GenericDataSafeClass::Decrypt(void *data, int size, unsigned int key, bool do_checksum)
{
	ds_assert((size % 4) == 0);
	unsigned int *data_ptr = (unsigned int*)data;

	if (do_checksum) {
		for (int i = 0 ; i < (size / 4) ; i++) {
			Checksum ^= *data_ptr;
			*data_ptr++ ^= key;
		}
	} else {
		for (int i = 0 ; i < (size / 4) ; i++) {
			*data_ptr++ ^= key;
		}
	}
}





/***********************************************************************************************
 * GenericDataSafeClass::Get_Random_List_For_Insertion -- Get a random list to insert an item  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Item type                                                                         *
 *                                                                                             *
 * OUTPUT:   List index                                                                        *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/25/2001 12:07PM ST : Created                                                            *
 *=============================================================================================*/
int GenericDataSafeClass::Get_Random_List_For_Insertion(int type)
{
	ThreadLockClass locker;

	random_list_contenders.Reset_Active();

	/*
	** Build a list of lists that contain the appropriate size data and have room for extra entries.
	*/
	for (int i=0 ; i<NumLists ; i++) {
		ds_assert(Safe[i] != NULL);
		if (Safe[i] != NULL && (Safe[i]->EntryType == type || Safe[i]->EntryType == -1) && Safe[i]->EntryCount < MAX_ENTRIES_PER_LIST) {
			random_list_contenders.Add(i);
		}
	}

	/*
	** If no lists are available then we need to create one.
	*/
	if (random_list_contenders.Count() == 0) {
		int safe_index = Create_Safe_List(type);
		ds_assert(safe_index != -1);
		ds_assert(Safe[safe_index] != NULL);
		return(safe_index);
	}

	/*
	** Choose a random list from the possible contenders.
	*/
	int pick = 0;
	if (random_list_contenders.Count() > 1) {
		pick = FreeRandom.Get_Int(0, random_list_contenders.Count()-1);
	}
	ds_assert(pick >= 0 && pick < random_list_contenders.Count());

	/*
	** If the list we picked is currently empty then we might need to do some fixup before we can use it.
	*/
	int list_index = random_list_contenders[pick];
	ds_assert(Safe[list_index] != NULL);
	if (Safe[list_index]->EntryType == -1 || Safe[list_index]->SafeList == NULL) {
		Safe[list_index]->EntryType = type;
		Safe[list_index]->SlopCount = 0;
	}

	return(list_index);
}



/***********************************************************************************************
 * GenericDataSafeClass::Create_Safe_List -- Create a single data safe list of specified type  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Type of list to create                                                            *
 *                                                                                             *
 * OUTPUT:   Index of list. -1 if unable to create                                             *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/25/2001 12:27PM ST : Created                                                            *
 *=============================================================================================*/
int GenericDataSafeClass::Create_Safe_List(int type)
{
	/*
	** Create a new list entry.
	*/
	DataSafeEntryListClass *safe_list = new DataSafeEntryListClass;
	ds_assert(safe_list->EntryCount == 0);
	ds_assert(safe_list->SafeList == NULL);
	safe_list->EntryType = type;
	safe_list->SlopCount = 0;

	/*
	** Add it to the list of data safes.
	*/
	ds_assert(NumLists < MAX_DATASAFE_LISTS);
	if (NumLists < MAX_DATASAFE_LISTS) {
		Safe[NumLists++] = safe_list;
		return(NumLists-1);
	}

	return(-1);
}



/***********************************************************************************************
 * GenericDataSafeClass::Random_Insertion -- Insert an entry at a random position in the list  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to data entry                                                                 *
 *           Index of list to add to                                                           *
 *           Type of data (size)                                                               *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/25/2001 12:34PM ST : Created                                                            *
 *=============================================================================================*/
void GenericDataSafeClass::Random_Insertion(DataSafeEntryClass *entry_ptr, int list, int /* type */, bool is_slop)
{
	ds_assert(Safe[list] != NULL);
	ds_assert(Safe[list]->EntryCount < MAX_ENTRIES_PER_LIST);
	ds_assert(Safe[list]->EntryType == type);

	/*
	** If this is the first entry to be added to the list then there is no possibility of random insertion. Just stick it
	** at the beginning.
	*/
	if (Safe[list]->EntryCount == 0) {
		entry_ptr->Next = entry_ptr;
		entry_ptr->Prev = entry_ptr;
		Safe[list]->SafeList = entry_ptr;
		Safe[list]->EntryCount++;
	} else {

		/*
		** Pick a random insertion point.
		*/
		int pick = 0;
		if (Safe[list]->EntryCount > 1) {
			pick = FreeRandom.Get_Int(0, Safe[list]->EntryCount-1);
		}

		/*
		** Find the item to insert ahead of.
		*/
		DataSafeEntryClass *walker = Safe[list]->SafeList;
		int skip = pick;
		while (skip) {
			walker = walker->Next;
			skip--;
		}

		/*
		** Fix up the list pointers to insert.
		*/
		entry_ptr->Prev = walker->Prev;
		walker->Prev->Next = entry_ptr;
		entry_ptr->Next = walker;
		walker->Prev = entry_ptr;

		if (pick == 0) {
			Safe[list]->SafeList = entry_ptr;
		}

		Safe[list]->EntryCount++;
	}
	Safe[list]->SlopCount += (is_slop ? 1 : 0);
}




/***********************************************************************************************
 * GenericDataSafeClass::Swap_Entries -- Swap two entries in memory (must be same type/size).  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to first entry                                                                *
 *           Ptr to second entry                                                               *
 *           Entry types (must be same type)                                                   *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: Entries must be same size and in the same list. If entries are moved to a         *
 *           different list then the owners handle will no longer work                         *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/25/2001 1:02PM ST : Created                                                             *
 *=============================================================================================*/
void GenericDataSafeClass::Swap_Entries(DataSafeEntryClass *first, DataSafeEntryClass *second, int type)
{
	ds_assert(first != NULL);
	ds_assert(second != NULL);
	ds_assert(type >= 0);

#ifdef	WWDEBUG
	NumSwaps++;
#endif	//WWDEBUG

	/*
	** Get pointers to the actual data rider part of the entry.
	*/
	void *first_data = ((char*)first) + sizeof(*first);
	void *second_data = ((char*)second) + sizeof(*second);

	/*
	** Swap the objects in memory. Just swap the data and the handle so that the next and prev pointers are still good
	*/
	if (first && second && first != second) {

		int size = Get_Type_Size(type);	// + sizeof(DataSafeEntryClass);

		/*
		** Optimise for small data types.
		*/
		if (size == 4) {

			/*
			** Convert to int pointers to make it easy to read.
			*/
			int *p1 = (int*) first_data;
			int *p2 = (int*) second_data;

			int temp = *p1;
			*p1 = *p2;
			*p2 = temp;

		} else {

			/*
			** Use a temp buffer on the stack to swap through.
			*/
			void *temp_buffer = _alloca(size);
			memcpy(temp_buffer, first_data, size);
			memcpy(first_data, second_data, size);
			memcpy(second_data, temp_buffer, size);
		}

		/*
		** Swap the handles.
		*/
		DataSafeHandleClass temp_handle = first->Handle;
		bool is_slop = first->IsSlop;

		first->Handle = second->Handle;
		first->IsSlop = second->IsSlop;

		second->Handle = temp_handle;
		second->IsSlop = is_slop;
	}
}








/***********************************************************************************************
 * GenericDataSafeClass::Remove_From_List -- Remove entry from the linked list and fix pointers*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    List number                                                                       *
 *           Pointer to entry in list                                                          *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/29/2001 10:54AM ST : Created                                                            *
 *=============================================================================================*/
void GenericDataSafeClass::Remove_From_List(int list, DataSafeEntryClass *entry_ptr)
{
	ds_assert(list >= 0);
	ds_assert(list < NumLists);
	ds_assert(Safe[list] != NULL);

	/*
	** We found the entry for this handle. We need to fix up the list pointers and delete the memory.
	*/
	if (Safe[list] && Safe[list]->EntryCount) {

		int data_size = Get_Type_Size(Safe[list]->EntryType);

		if (Safe[list]->EntryCount > 1) {
			entry_ptr->Prev->Next = entry_ptr->Next;
			entry_ptr->Next->Prev = entry_ptr->Prev;

			/*
			** If this entry was at the head of the list then we need to adjust the head pointer.
			*/
			if (Safe[list]->SafeList == entry_ptr) {
				Safe[list]->SafeList = entry_ptr->Next;
			}
		} else {
			Safe[list]->SafeList = NULL;
			Safe[list]->EntryType = -1;
		}

		Safe[list]->EntryCount--;
		if (entry_ptr->IsSlop) {
			Safe[list]->SlopCount--;
		}

		/*
		** Muss up the memory so the data value isn't hanging around.
		*/
		memset(entry_ptr, FreeRandom.Get_Int(0, 255), sizeof(*entry_ptr) + data_size);
	}
}



/***********************************************************************************************
 * GenericDataSafeClass::Shuffle -- Move stuff around in the safe, maybe change the key        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Forced - set if we should shuffle now - even if it's not time yet                 *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: Sloooooooow                                                                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/25/2001 9:12PM ST : Created                                                             *
 *=============================================================================================*/
void GenericDataSafeClass::Shuffle(bool forced)
{
	ThreadLockClass locker;

#ifndef FIXED_KEY

	/*
	** Only check the time every n calls.
	*/
	static unsigned int _calls = 0;
	_calls++;
	if (_calls < DATASAFE_TIME_CHECK_CALLS) {
		return;
	}
	_calls = 0;

	/*
	** Locals.
	*/
	unsigned int new_key;
	unsigned int mod_key;
	int i,j;

	/*
	** We should only do this once in a while
	*/
	unsigned int time = TIMEGETTIME();

	if (forced || time < ShuffleDelay || (time | ShuffleDelay) == 0 || (time - ShuffleDelay) > SHUFFLE_TIME) {
		//WWDEBUG_SAY(("Data Safe: Performing data shuffle and re-key\n"));

#ifdef WWDEBUG
		NumShuffles++;
#endif	//WWDEBUG

		/*
		** Calculate a new key. Lifed from old datasafe code.
		*/
		/*
		** Change keys
		** Given: encrypted item a, old key b, new key c
		** To re-key, the normal algorithm is (a ^ b) ^ c
		** To hide what we're doing, we're going to do
		** this:  a ^ ( b ^ c )
		*/

		/*
		** Clear keys.
		*/
		mod_key ^= mod_key;
		new_key ^= new_key;

		if (SimpleKey == 0) {
			/*
			** Do some error stuff.
			*/
		}

		/*
		** XOR in old key.
		*/
		mod_key ^= SimpleKey;

		/*
		** Generate a new key.
		*/
		new_key = TIMEGETTIME();
		new_key ^= FreeRandom.Get_Int(0, 0xffffffff);

		/*
		** Reset the checksum. Can't keep a running checksum if we re-key
		*/
		Checksum = ~new_key;

		/*
		** Xor in the new key
		*/
		mod_key ^= new_key;

		/*
		** Now, mod_key contains the "modifier key" used to rekey items. We will use it after we have swapped some stuff around.
		*/
		for (i=0 ; i<NumLists ; i++) {
			ds_assert(Safe[i] != NULL);
			if (Safe[i] != NULL && Safe[i]->EntryCount > 1) {
				for (j=0 ; j < std::max(Safe[i]->EntryCount / 3, 1) ; j++) {

					/*
					** This code has been replaced by the simplified version below to avoid a compiler bug in release mode.
					*/
					//DataSafeEntryClass *first = Get_Entry_By_Index(i, FreeRandom.Get_Int(0, Safe[i]->EntryCount - 1));
					//DataSafeEntryClass *second = Get_Entry_By_Index(i, FreeRandom.Get_Int(0, Safe[i]->EntryCount - 1));

					int first_index = FreeRandom.Get_Int(0, Safe[i]->EntryCount - 1);
					int second_index = FreeRandom.Get_Int(0, Safe[i]->EntryCount - 1);
					DataSafeEntryClass *first = Get_Entry_By_Index(i, first_index);
					DataSafeEntryClass *second = Get_Entry_By_Index(i, second_index);

					if (first && second && first != second) {
						Swap_Entries(first, second, Safe[i]->EntryType);
					}
				}
			}
		}

		/*
		** Clear the old key.
		*/
		SimpleKey ^= SimpleKey;

		/*
		** Now, as if that wasn't slow enough, we have to go through and re-key *everything*.
		*/

		/*
		** Loop through all safe lists.
		*/
		for (i=0 ; i<NumLists ; i++) {
			ds_assert(Safe[i] != NULL);
			if (Safe[i] != NULL && Safe[i]->EntryCount > 0) {

				/*
				** Get the pointer to the first entry in the list.
				*/
				DataSafeEntryClass *entry_ptr = Safe[i]->SafeList;

				/*
				** Loop through all entries in this list.
				*/
				for (j=0 ; j < Safe[i]->EntryCount ; j++) {

					/*
					** Re-key the entry.
					*/
					entry_ptr->Handle = entry_ptr->Handle ^ mod_key;
					Checksum ^= entry_ptr->Handle;

					void *data_ptr = ((char *)entry_ptr) + sizeof(*entry_ptr);
					Encrypt(data_ptr, Get_Type_Size(Safe[i]->EntryType), mod_key, true);

					/*
					** Next entry.
					*/
					entry_ptr = entry_ptr->Next;
				}
			}
		}

		/*
		** Set the key.
		*/
		SimpleKey ^= new_key;

		/*
		** Reset the delay.
		*/
		ShuffleDelay = TIMEGETTIME();

		/*
		** Check safe integrity.
		*/
		Security_Check();
	}
#endif	//FIXED_KEY
}



/***********************************************************************************************
 * GenericDataSafeClass::Get_Handle_ID -- Get the next available handle ID in the given list.  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    List number                                                                       *
 *                                                                                             *
 * OUTPUT:   Handle ID                                                                         *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2001 8:56PM ST : Created                                                              *
 *=============================================================================================*/
int GenericDataSafeClass::Get_Handle_ID(int list)
{
	/*
	** Asserts.
	*/
	ds_assert(Safe[list] != NULL);

	/*
	** If the list isn't allocated then we are out of luck.
	*/
	if (Safe[list] != NULL) {

		/*
		** Go through the handle id usage table looking for a free spot.
		*/
		for (int i=0 ; i<MAX_ENTRIES_PER_LIST ; i++) {
			if (Safe[list]->HandleIDUsage[i] == 0) {
				Safe[list]->HandleIDUsage[i] = 1;
				return(i);
			}
		}
	}

	/*
	** Error. No free space or list not allocated.
	*/
	return(0);
}




/***********************************************************************************************
 * GenericDataSafeClass::Free_Handle_ID -- Return a handle ID to the available pool            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Safe list number                                                                  *
 *           Handle ID                                                                         *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2001 8:59PM ST : Created                                                              *
 *=============================================================================================*/
void GenericDataSafeClass::Free_Handle_ID(int list, int id)
{
	/*
	** Asserts.
	*/
	ds_assert(Safe[list] != NULL);
	ds_assert(id >= 0 && id < MAX_ENTRIES_PER_LIST);

	/*
	** If the list isn't allocated then we are out of luck.
	*/
	if (Safe[list] != NULL) {

		/*
		** It's easy, we just zero out the array element of the ID we no longer need.
		*/
		Safe[list]->HandleIDUsage[id] = 0;
	}
}


/***********************************************************************************************
 * GenericDataSafeClass::Say_Security_Fault -- Security fault display action                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/8/2001 2:25PM ST : Created                                                              *
 *=============================================================================================*/
void GenericDataSafeClass::Say_Security_Fault(void)
{
	WideStringClass text(TRANSLATION(IDS_DATASAFE_DETECTED_TAMPERING), true);
	if (cNetwork::I_Am_Server()) {
		text += U_CHAR("Host");
		cScTextObj *event_obj = new cScTextObj;
		event_obj->Init(text, TEXT_MESSAGE_PUBLIC, false, HOST_TEXT_SENDER, -1);
	} else {
		cCsTextObj *message = new cCsTextObj;
		cPlayer *me = cPlayerManager::Find_Player(cNetwork::Get_My_Id());
		if (me) {
			text += me->Get_Name();
		}
		message->Init(text, TEXT_MESSAGE_PUBLIC, cNetwork::Get_My_Id(), -1);
	}
}


#ifdef THREAD_SAFE_DATA_SAFE

/***********************************************************************************************
 * GenericDataSafeClass::Lock -- Get the safe mutex.                                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/6/2001 3:02PM ST : Created                                                              *
 *=============================================================================================*/
inline void GenericDataSafeClass::Lock(void)
{
	int deadlock = WaitForSingleObject(GenericDataSafeClass::SafeMutex, 10 * 1000);
	if (deadlock == WAIT_TIMEOUT) {
		WWDEBUG_SAY(("ERROR: Data Safe: Timeout waiting for data safe mutex\n"));
		ds_assert(deadlock != WAIT_TIMEOUT);
	}
}


/***********************************************************************************************
 * GenericDataSafeClass::Unlock -- Release the safe mutex.                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/6/2001 3:03PM ST : Created                                                              *
 *=============================================================================================*/
inline void GenericDataSafeClass::Unlock(void)
{
	ReleaseMutex(GenericDataSafeClass::SafeMutex);
}

#endif //THREAD_SAFE_DATA_SAFE



/***********************************************************************************************
 * GenericDataSafeClass::Print_Safe_Stats_To_Debug_Output -- Print stats with OutputDebugStringA*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/16/2001 2:28PM ST : Created                                                             *
 *=============================================================================================*/
void GenericDataSafeClass::Print_Safe_Stats_To_Debug_Output(void)
{
#ifdef WWDEBUG
	if (cDevOptions::LogDataSafe.Is_True()) {
		char temp[8192];
		Dump_Safe_Stats(temp, 8192);
		WWDEBUG_SAY((temp));
	}
#endif	//WWDEBUG
}


/***********************************************************************************************
 * GenericDataSafeClass::Dump_Safe_Stats - Dump safe stats into a buffer                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to buffer to receive stats                                                    *
 *           Size of buffer                                                                    *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/16/2001 2:12PM ST : Created                                                             *
 *=============================================================================================*/
void GenericDataSafeClass::Dump_Safe_Stats([[maybe_unused]] char *dump_buffer, [[maybe_unused]] int buffer_size)
{
#ifdef	WWDEBUG

#define UPDATE_PTR									\
 	chars += strlen(dump_ptr);					\
	dump_ptr = dump_buffer + chars;			\
	ds_assert(buffer_size > chars);				\


	/*
	** Asserts.
	*/
	ds_assert(dump_buffer != NULL);
	ds_assert(buffer_size >= 1024);
	ds_assert(!IsBadWritePtr(dump_buffer, buffer_size));

	/*
	** Make a copy of the buffer ptr.
	*/
	int chars = 0;
	char *dump_ptr = dump_buffer;

	/*
	** Precalculate some stats.
	*/
	unsigned int time = TIMEGETTIME() - LastDump;
	LastDump = TIMEGETTIME();
	time = time / 1000;
	int fetches_per_second = 0;
	int swaps_per_second = 0;
	if (time) {
		fetches_per_second = NumFetches / time;
		swaps_per_second = NumSwaps / time;
	}
	NumFetches = 0;
	NumSwaps = 0;

	/*
	** Dump basic datasafe stats into the buffer.
	*/
	sprintf(dump_ptr, "Data Safe Stats\n\n");
	UPDATE_PTR;

	sprintf(dump_ptr, "  Lists              : %d\n", NumLists);
	UPDATE_PTR;
	sprintf(dump_ptr, "  Slop Entries       : %d\n", SlopCount);
	UPDATE_PTR;
	sprintf(dump_ptr, "  Safe Key           : %08X\n", SimpleKey);
	UPDATE_PTR;
	sprintf(dump_ptr, "  Handle Key         : %08X\n", HandleKey);
	UPDATE_PTR;
	sprintf(dump_ptr, "  Checksum           : %08X\n", Checksum);
	UPDATE_PTR;
	sprintf(dump_ptr, "  Known Types        : %d\n", TypeListCount);
	UPDATE_PTR;
	sprintf(dump_ptr, "  Security Checks    : %d\n", NumSecurityChecks);
	UPDATE_PTR;
	sprintf(dump_ptr, "  Shuffles           : %d\n", NumShuffles);
	UPDATE_PTR;
	sprintf(dump_ptr, "  Fetches per second : %d\n", fetches_per_second);
	UPDATE_PTR;
	sprintf(dump_ptr, "  Swaps per second   : %d\n", swaps_per_second);
	UPDATE_PTR;
	sprintf(dump_ptr, "  CRC Errors         : %d\n", CRCErrors);
	UPDATE_PTR;

	/*
	** Count the number of items currently stored in the data safe.
	*/
	int count = 0;
	unsigned int bytes = 0;
	for (int i=0 ; i<NumLists ; i++) {
		ds_assert(Safe[i] != NULL);
		if (Safe[i] != NULL) {
			if (Safe[i]->EntryCount) {
				count += Safe[i]->EntryCount;
				bytes += (sizeof(DataSafeEntryClass) + Get_Type_Size(Safe[i]->EntryType)) * Safe[i]->EntryCount;
			}
		}
	}

	/*
	** Print out the usage stats.
	*/
	sprintf(dump_ptr, "\n  Safe contains %d objects consuming %d bytes\n", count, bytes);
	UPDATE_PTR;

	/*
	** Print out the percentage of safe capacity in use.
	*/
	unsigned int percent = (count * 100) / (MAX_DATASAFE_LISTS * MAX_ENTRIES_PER_LIST);
	sprintf(dump_ptr, "\n  Safe is %d percent full\n\n", percent);
	UPDATE_PTR;

#endif	//WWDEBUG
}