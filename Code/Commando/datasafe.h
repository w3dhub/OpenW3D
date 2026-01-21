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
 *                     $Archive:: /Commando/Code/Commando/datasafe.h                          $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 1/31/02 12:13p                                              $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef _DATASAFE_H
#define _DATASAFE_H

/*
** Adapted from original code by Jeff Brown
**
**   //=============================================================================
**   // DataSafe.h
**   // Public header file for the datasafe
**   // Author: Jeff Brown
**   // Date: Apr 16, 1999
**   //=============================================================================
**
**
**
**
** Class Overview. ST - 7/11/2001 8:19PM
**
**	DataSafeHandleClass.
**		Used to manage handles to items stored in the data safe.
**
**	DataSafeEntryClass and DataSafeEntryDataClass<T>.
**		Represent a single item as stored in the data safe. DataSafeEntryClass has the header
** 		information used to retrieve the item from the safe and the derived DataSafeEntryDataClass
**		also includes the actual type specific data payload.
**
**	DataSafeEntryListClass
**		This is a manager class for a linked list of DataSafeEntryDataClass<T>'s. Only the same
**		type of DataSafeEntryDataClass<T> classes can currently be linked together and exist in
**		the same list.
**
**	DataSafeEntryTypeClass
**		Every object put into the data safe is allocated a type ID so that like types (with the
**		same ID) can be identified for linking and swapping purposes. DataSafeEntryTypeClass is
**		a covenient holder for type information.
**
**	GenericDataSafeClass and DataSafeClass<T>
**     The safe itself consists of one static GenericDataSafeClass and many DataSafeClass<T>'s -
**		one DataSafeClass<T> for every type of data we want to store in the data safe.
**		Also included is an array of DataSafeEntryListClass's that act as pointers to the data safe
**		entries.
**
**
**	SafeDataClass
**		This is a convenience class used to automate insertion and removal of items and provide
**		a set of operators for common types. Using this class, adding existing data to the data safe
**		can be as simple as just changing how it's declared. e.g.
**
**		int SomeData;
**
**			becomes
**
**		SafeDataClass<int> SomeData;
**
**			There are also some macros provided to make declaration easier so you could also just say
**
**		safe_int SomeData;
**
** 		Currently, only a subset of operators are defined but more can easily be added as needed.
** 		Some changes to code will be required where operators are not available. For example
**
** 		SomeData += 10;
**
** 			would be a problem because the += operator is not defined. However the 'int' operator, the
**			= operator and the conversion operator () are defined so you could change the code to read
**
** 		SomeData = (int)SomeData + 10;
**
** 			or
**
** 		SomeData = SomeData() + 10;
**
**		You can pretty much throw anything into the data safe using this class to automate isertion
**		and retrieval. Some examples.
**
**		safe_float		HideyFloat;
**     safe_double		CantFindMe[256];
**		SafeDataClass<SomeNamedClass> ASafeClass;
**		SafeDataClass<SomeOtherType> ASafeType;
**
**		For types without existing macros, you will have to declare a new DataSafeClass<T> once for
**		each type in DataSafe.cpp using the DECLARE_DATA_SAFE macro. So in the above example, you
**		would have to add the following lines in DataSafe.cpp to allow storage of the new types
**
**		DECLARE_DATA_SAFE(SomeNamedClass);
**		DECLARE_DATA_SAFE(SomeOtherType);
**
**
**
**
**	Known external dependencies.
**		DynamicVectorClass
**		TICKS_PER_SECOND = 1000
**		FreeRandom - Renegade random number generator.
**		WWDEBUG_SAY - Equivalent to DebugString in C&C. Just a formatted debug output.
**
*/

/*
** Includes.
*/
#ifndef 	_ALWAYS_H
#include "always.h"
#include <assert.h>
#include <malloc.h>
#include <memory.h>
#ifdef _WINDOWS
#include <windows.h>
#endif
#endif	//_ALWAYS_H

#ifndef 	_TIMEMGR_H
#include	"timemgr.h"
#endif	//_TIMEMGR_H

#ifndef	_WWDEBUG_H
#include "wwdebug.h"
#endif	//_WWDEBUG_H

//#ifndef	MMSYS_H
//#include "mmsys.h"
//#endif	//MMSYS_H
#include "systimer.h"

/*
** Use Renegade assert.
*/
#ifdef WWASSERT
#define ds_assert WWASSERT
#else	//WWASSERT
#define ds_assert assert
#endif	//WWASSERT


/*
** TEMP. Profile builds don't have asserts.
*/
#ifndef DEBUG
#undef ds_assert
#define ds_assert(x)
#endif //DEBUG


/*
** Enable this define to make the datasafe thread safe (there is a performance penalty).
*/
//#define THREAD_SAFE_DATA_SAFE

/*
** Number of linked lists we can create for data safe items. Only like sized items can be stored in the same list.
*/
#define MAX_DATASAFE_LISTS	8192

/*
** The maximum number of different data types that can be stored in the data safe. This should normally fit into a byte for
** handle encoding reasons.
*/
#define MAX_DATASAFE_TYPES 128

/*
** Max number of entries in a single list before we have to create a second list for items of the same size. The smaller this
** value, the quicker we will be able to retrieve data from the safe. The larger we make it, the more secure the data safe will
** be.
*/
#define MAX_ENTRIES_PER_LIST	16

/*
** How often the data safe entries get shuffled around and re-keyed. This is the delay between each shuffle.
*/
#define SHUFFLE_TIME (TICKS_PER_SECOND * 7)

/*
** The delay between each integrity check on the data safe. This is a slow operation.
*/
#define SECURITY_CHECK_TIME (TICKS_PER_SECOND * 15)

/*
** The number of copies of objects of any one type that can exist in an unencrypted state. These copies are only used as return
** values and changes to the copies can never be propagated back into the safe.
*/
#define MAX_OBJECT_COPIES 8

/*
** Sentinel values. Used to see if someone just zeroes the whole safe or tampered with the get function.
*/
#define SENTINEL_ONE 0x29874362
#define SENTINEL_TWO 0x0bad0bad

#define DATASAFE_TIME_CHECK_CALLS	100000

/*
** Defines to allow convenient use of built-in types inside the data safe.
**
*/
#ifdef PARAM_EDITING_ON

#define safe_int int
#define safe_unsigned_int unsigned int

#define safe_long int
#define safe_unsigned_long unsigned int

#define safe_float float

#define safe_double double

#else //PARAM_EDITING_ON

#define safe_int SafeDataClass<int>
#define safe_unsigned_int SafeDataClass<unsigned int>

#define safe_long SafeDataClass<int>
#define safe_unsigned_long SafeDataClass<unsigned int>

#define safe_float SafeDataClass<float>

#define safe_double SafeDataClass<double>

#endif	//PARAM_EDITING_ON


/*
** Make sure we pulled in the right timer defines.
*/
#if TICKS_PER_SECOND != 1000
#error Code assumes TICKS_PER_SECOND == 1000
#endif

/*
** Data safe handle.
**
** This consists of a linked list number, an entry identifier and a type.
**
** The whole shooting match can just be treated like an int.
**
*/
class DataSafeHandleClass
{
	public:

		/*
		** Consturctor.
		*/
		DataSafeHandleClass(int val = 0) {
			Handle.Whole.WholeHandle = val;
		};

		/*
		** Validity check.
		*/
		bool Is_Valid(void) const {
			return(Handle.Whole.WholeHandle != -1);
		};

		/*
		** Convenience functions and operators to handle setting or getting a handle using an int.
		*/
		inline int operator = (const int &val) {
			Handle.Whole.WholeHandle = val;
			return(val);
		};

		inline operator int (void) {
			return(Handle.Whole.WholeHandle);
		}

		inline operator int (void) const {
			return(Handle.Whole.WholeHandle);
		}

		inline int &operator () (void) {
			return(Handle.Whole.WholeHandle);
		}

		/*
		** Handle data.
		*/
		union tHandle {
			struct {
				/*
				** All the handle components conveniently together in an int.
				*/
				int WholeHandle;
			} Whole;

			struct {

				/*
				** ID is basically the index of the item in the linked list. It's a little more complex
				** than that since an ID can be re-used after the original user is removed from the
				** list but the whole list is searched for a matching ID when retrieving anyway.
				*/
				unsigned char ID;

				/*
				** The type of object this handle represents.
				*/
				unsigned char Type;

				/*
				** The list number where we can find this object.
				*/
				unsigned short List;
			} Part;

		} Handle;
};

/*
** Known packing is required here otherwise the default alignment is the same as the largest member
** of the structure. When combined with an expansion of DataSafeEntryDataClass<T> for doubles, the
** packing alignment becomes 8 (the size of a double) thus the offset of Data is not always as expected.
** i.e. you can no longer use sizeof(DataSafeEntryClass) to get the offset of the Data member of
** the derived class. This also prevents bloating (due to the bool member) when using large expansions
** for 'T'
*/
#pragma pack(push)
#pragma pack(4)

/*
**
** This class represents an internal data safe entry minus the actual data. In practice, this class will never be used
** except to refer to a DataSafeEntryDataClass<T> which actually contains the type specific data.
**
*/
class DataSafeEntryClass
{

	public:

		/*
		** Identifying handle.
		*/
		DataSafeHandleClass Handle;

		/*
		** Linked list next and prev.
		**
		** Note: These can point to a DataSafeEntryDataClass expanded for any type.
		*/
		DataSafeEntryClass *Next;
		DataSafeEntryClass *Prev;

		/*
		** Size of data.
		*/
		unsigned int Size;

		/*
		** Is this a slop (fake, to allow swapping with only 1 real entry) entry?
		*/
		bool IsSlop;
};


/*
** This is a complete internal data safe entry including the header and the encrypted data.
**
*/
template<class T>
class DataSafeEntryDataClass : public DataSafeEntryClass
{
	public:
		T Data;
};

#pragma pack(pop)



/*
** This class represents a single data safe entry list. There can be mutiple lists in a data safe but all entries in a single
** list must be of the same type.
*/
class DataSafeEntryListClass
{
	public:

		/*
		** Constructor.
		*/
		DataSafeEntryListClass(void) {
			SafeList = NULL;
			EntryCount = 0;
			EntryType = -1;
			SlopCount = 0;
			memset(HandleIDUsage, 0, sizeof(HandleIDUsage));
		};

		/*
		** Pointer to the head of the list.
		*/
		DataSafeEntryClass *SafeList;

		/*
		** Number of entries in the list.
		*/
		int EntryCount;

		/*
		** Type of entry in this list.
		*/
		int EntryType;

		/*
		** Number of slop entries in this list.
		*/
		int SlopCount;

		/*
		** Handle IDs used in this list.
		*/
		char HandleIDUsage[MAX_ENTRIES_PER_LIST];
};


/*
**
** This class describes a type of entry in the data safe. It's important to keep track of the types of objects we add since
** only similar types can be swapped in memory. This class is just a convenient container to help us match type codes with
** unique IDs.
**
**
*/
class DataSafeEntryTypeClass
{
	public:

		DataSafeEntryTypeClass(void) {
			TypeCode = 0;
			ID = -1;
			Size = 0;
		};


		/*
		** A unique number used to match and assign type IDs. This can come from anywhere as long as it's different for every type.
		*/
		unsigned int TypeCode;

		/*
		** This is the user friendly ID that is stored along with entries in the data safe and returned in the handle that's
		** given out when something is added to the safe.
		** It's also the index into the type list.
		*/
		int ID;

		/*
		** The size of this type of object. Must be a multiple of 4 bytes for the data safe. (This is redundant now that we
		** have a type/size lookup table).
		*/
		int Size;
};







/*
**  GenericDataSafeClass
**
**		This class represents a single data safe. It can only function as the base class to one or more instances of
** 	DataSafeClass<T>.
**
**
** 	This class functions as a base class for all the templated 'DataSafeClass'es so that they can all share a single set of
**		static data. Normally, static data in a templated class is duplicated for each expansion of the class.
**
**    Even though there may be many instances and expansions of the derived DataSafeClass<T> there is only ever one global
**		data safe and that is represented by this class.
**
*/
class GenericDataSafeClass
{
	public:

		/*
		** Constructor.
		*/
		GenericDataSafeClass(void);

		/*
		** Query functions.
		*/
		static DataSafeEntryClass * Get_Entry(DataSafeHandleClass handle);
		static int Get_Entry_Type(DataSafeHandleClass handle);

		/*
		** System timer reset.
		*/
		static void Reset_Timers(void);

		/*
		** Thread safety. This is normally handled automatically but if you need to manually lock the safe for a period of time
		** to do a read/hold/write then this is the way to do it.
		*/
#ifdef THREAD_SAFE_DATA_SAFE
		static void Lock(void);
		static void Unlock(void);
#endif //THREAD_SAFE_DATA_SAFE

		/*
		** Stats, debug helpers.
		*/
		static void Dump_Safe_Stats(char *dump_buffer, int buffer_size);
		static void Print_Safe_Stats_To_Debug_Output(void);
		static void Reset(void);


#ifdef THREAD_SAFE_DATA_SAFE
		static void Set_Preferred_Thread(unsigned int){};
#else //THREAD_SAFE_DATA_SAFE
		static void Set_Preferred_Thread(unsigned int thread_id) {PreferredThread = thread_id;};
#endif //THREAD_SAFE_DATA_SAFE

	protected:

		/*
		** Cleanup.
		*/
		void Shutdown(void);

		/*
		** Query functions.
		*/
		static DataSafeEntryClass * Get_Entry_By_Index(int list, int index);

		/*
		** Security
		*/
		static void Shuffle(bool forced = false);
		static void Swap_Entries(DataSafeEntryClass *first, DataSafeEntryClass *second, int type);
		static void Encrypt(void *data, int size, unsigned int key = SimpleKey, bool do_checksum = true);
		static void Decrypt(void *data, int size, unsigned int key = SimpleKey, bool do_checksum = true);
		static void Mem_Copy_Encrypt(void *dest, void *src, int size, bool do_checksum);
		static void Mem_Copy_Decrypt(void *dest, void *src, int size, bool do_checksum);
		static __forceinline void Security_Check(void);
		static __forceinline void Security_Fault(void);
		static void Say_Security_Fault(void);

		/*
		** Insertion support.
		*/
		static int Get_Random_List_For_Insertion(int type);
		static void Random_Insertion(DataSafeEntryClass *entry, int list, int type, bool is_slop = false);
		static int Create_Safe_List(int type);
		static int Get_Handle_ID(int list);

		/*
		** Deletion support. Just used to make sure we are doing type safe deletion.
		*/
		static void Remove_From_List(int list, DataSafeEntryClass *entry_ptr);
		static void Free_Handle_ID(int list, int id);

		/*
		** Type identification.
		*/
		static inline int Get_Type_Size(int type);

#ifdef THREAD_SAFE_DATA_SAFE
		/*
		** Thread safety.
		*/
		class ThreadLockClass
		{
			public:
				/*
				** Constructor. Grabs the mutex.
				*/
				inline ThreadLockClass(void) {
					int deadlock = WaitForSingleObject(GenericDataSafeClass::SafeMutex, 10 * 1000);
					if (deadlock == WAIT_TIMEOUT) {
						WWDEBUG_SAY(("Data Safe: Timeout waiting for data safe mutex\n"));
						ds_assert(deadlock != WAIT_TIMEOUT);
					}
				};

				/*
				** Destructor, releases the mutex.
				*/
				inline ~ThreadLockClass(void) {
					ReleaseMutex(GenericDataSafeClass::SafeMutex);
				};
		};
#else //THREAD_SAFE_DATA_SAFE

		/*
		** If thread safety is not required, this class compiles out to nothing when used in release mode.
		*/
		class ThreadLockClass
		{
			public:
				__forceinline ThreadLockClass(void) {
#ifdef WWDEBUG
					if (GenericDataSafeClass::PreferredThread != GetCurrentThreadId()) {
						WWDEBUG_SAY(("DATASAFE.H - PreferredThread = %08X, GetCurrentThreadId() == %08X\n", GenericDataSafeClass::PreferredThread, GetCurrentThreadId()));
					}
					ds_assert(GenericDataSafeClass::PreferredThread == GetCurrentThreadId());
#endif //WWDEBUG
				};
		};

		static unsigned int PreferredThread;
#endif //THREAD_SAFE_DATA_SAFE

		friend ThreadLockClass;

		/*
		** Static data. We only need one each of these, regardless of how many DataSafeClass 'types' there are.
		**
		**
		**
		*/

		/*
		** Simple key value used for xoring.
		*/
		static unsigned int SimpleKey;

		/*
		** Key used for encrypting handles.
		*/
		static unsigned int HandleKey;

		/*
		** Number of valid entries in the Safe list.
		*/
		static int NumLists;

		/*
		** Array of pointers to data lists. We can't use a dynamic vector class here because the handles we give out include
		** an index into this array. Dynamic vector class moves entries around to remove gaps in the array which would leave
		** the handles pointing to the wrong element.
		*/
		static DataSafeEntryListClass *Safe[MAX_DATASAFE_LISTS];

		/*
		** Integrity check.
		*/
		static unsigned int Checksum;

		/*
		** Shuffle delay.
		*/
		static unsigned int ShuffleDelay;

		/*
		** Security check delay.
		*/
		static unsigned int SecurityCheckDelay;

		/*
		** List of types that are stored in the data safe.
		*/
		static DataSafeEntryTypeClass TypeList[MAX_DATASAFE_TYPES];
		static int TypeListCount;

		/*
		** Mutex to ensure thread safety.
		*/
#ifdef THREAD_SAFE_DATA_SAFE
		static HANDLE SafeMutex;
#endif //THREAD_SAFE_DATA_SAFE

		/*
		** Sentinels. Can't use 'SafeDataClass' as it's constructor adds data to the safe - i.e. us.
		*/
		static DataSafeHandleClass SentinelOne;
		static DataSafeHandleClass SentinelTwo;

		/*
		** Number of CRC errors so we can report it in the game results.
		*/
		static int CRCErrors;


		/*
		** Statistics - debug only.
		*/
#ifdef WWDEBUG
		static unsigned int LastDump;
		static int NumSwaps;
		static int NumFetches;
		static int SlopCount;
		static int NumSecurityChecks;
		static int NumShuffles;
#endif	//WWDEBUG

};



/*
**  DataSafeClass<T>
**
** 	This templated class is used to provide type specific functionality in the data safe. There must be at least one instance
**		of this class for each type of data we want to store in the safe. This, along with 'GenericDataSafeClass' represents
**		a single global data safe.
**
*/
template<class T>
class DataSafeClass : public GenericDataSafeClass
{

	public:
		DataSafeClass(T *bogus_ptr = NULL, int slopcount = 3);
		~DataSafeClass(void);

		/*
		** Query functions.
		*/
		static bool Get(DataSafeHandleClass handle, T* &item);
		static inline bool In_Return_List(T *ptr) {
			if (ptr) {
				void *temp = (void*)ptr;
				if (temp >= &ReturnList[0][0] && temp < &ReturnList[MAX_OBJECT_COPIES][0]) {
					if (((unsigned int) temp - (unsigned int)(&ReturnList[0][0])) % sizeof(T) == 0) {
						return(true);
					}
				}
			}
			return(false);

		};

		/*
		** Set functions.
		*/
		static bool Set(DataSafeHandleClass handle, T *data);

		/*
		** Add functions.
		*/
		static DataSafeHandleClass Add_Entry(T &value, bool is_slop = false);

		/*
		** Removal functions.
		*/
		static void Delete_Entry(DataSafeHandleClass handle);


	private:

		/*
		** Type identification.
		*/
		static int Get_Type_ID(unsigned int type_code, int size);
		static uintptr_t Get_Type_Code(void);

		/*
		** Type of this DataSafe.
		*/
		static int Type;

		/*
		** Min slop entries for this type of safe.
		*/
		static int MinSlop;

		/*
		** Copies of items in the data safe. Only used to return copies of items in the safe.
		**
		** The number of elements in this list is the number of times you can call the 'Get' function before you start to
		** invalidate the data you got 'n' calls ago. For this reason, you should always make a copy of data retrieved from
		** the safe if you plan to use it more than once.
		*/
		static char ReturnList[MAX_OBJECT_COPIES][sizeof(T)];
		static int ReturnIndex;
};




/*
**
**  Macros to help in declaring data safe types.
**
**
**
*/
#define DECLARE_DATA_SAFE(type) 																\
	DataSafeClass<type> DataSafe##type = {};														\
	template<> int DataSafeClass<type>::Type = 0;															\
	template<> char DataSafeClass<type>::ReturnList[MAX_OBJECT_COPIES][sizeof(type)] = {};		\
	template<> int DataSafeClass<type>::ReturnIndex = 0;													\
	template<> int DataSafeClass<type>::MinSlop = 0;





/*
**
** SafeDataClass.
**
** This class is for convenience when using the data safe. Just expand this class for the type you want to add to the data safe
** and it will keep track of the handle and automate the adding in, and removal of, data in the safe.
**
*/
extern char ErrorVal[1024];
template<class T>
class SafeDataClass
{

	public:

		/*
		** Constructor, destructor.
		*/
		SafeDataClass(void);
		SafeDataClass(T data);
		~SafeDataClass(void);

		/*
		** Copy constructor. This is a bad thing to call.
		*/
		SafeDataClass(SafeDataClass &) {
			ds_assert(false);
		};

		/*
		** Helper functions for data safe class storage.
		*/
		inline T* Get_Ptr(void) const;
		inline bool Commit(T* data_ptr) const;

		/*
		** Operators.
		*/
		inline T &operator () (void) const;

		inline T &operator = (T const &data);
		inline T &operator = (SafeDataClass<T> &safedata);

		inline operator int(void) const;
		inline operator unsigned int(void) const;
		inline operator float(void) const;
		inline operator double(void) const;

		inline bool operator == (T const &data);
		inline bool operator == (SafeDataClass<T> &safedata);

		inline bool operator != (T const &data);
		inline bool operator != (SafeDataClass<T> &safedata);

		inline bool operator > (T const &data);
		inline bool operator > (SafeDataClass<T> &safedata);

		inline bool operator >= (T const &data);
		inline bool operator >= (SafeDataClass<T> &safedata);

		inline bool operator < (T const &data);
		inline bool operator < (SafeDataClass<T> &safedata);

		inline bool operator <= (T const &data);
		inline bool operator <= (SafeDataClass<T> &safedata);

		inline T &operator + (T const &data);
		inline T &operator + (SafeDataClass<T> &safedata);

		inline T &operator += (T const &data);
		inline T &operator += (SafeDataClass<T> &safedata);

		inline T &operator - (T const &data);
		inline T &operator - (SafeDataClass<T> &safedata);

		inline T &operator -= (T const &data);
		inline T &operator -= (SafeDataClass<T> &safedata);

		inline T &operator * (T const &data);
		inline T &operator * (SafeDataClass<T> &safedata);

		inline T &operator *= (T const &data);
		inline T &operator *= (SafeDataClass<T> &safedata);

		inline T &operator / (T const &data);
		inline T &operator / (SafeDataClass<T> &safedata);

		inline T &operator /= (T const &data);
		inline T &operator /= (SafeDataClass<T> &safedata);

		inline T &operator ++ (void);
		inline T &operator -- (void);

		inline T &operator ++ (int);
		inline T &operator -- (int);

		/*
		** Query and set functions.
		*/
		DataSafeHandleClass Get_Handle(void) {return(Handle);};
		T Get(void);
		void Set(T);


	private:
		/*
		** This is the handle that represents this classes data in the datasafe.
		*/
		DataSafeHandleClass Handle;

#ifdef WWDEBUG
		T DebugData;
#endif //WWDEBUG


};












/***********************************************************************************************
**
**
**
** 		File separation.
**
**
*/











/*
** Simple functions, suitable for inline expansion.
**
**
**
**
*/


/*
**
**
**    GenericDataSafeClass functions.
**
**
*/

/***********************************************************************************************
 * GenericDataSafeClass::Get_Type_Size -- Get the size of this type                            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Type ID of object                                                                 *
 *                                                                                             *
 * OUTPUT:   Size of object                                                                    *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/27/2001 12:47PM ST : Created                                                            *
 *=============================================================================================*/
inline int GenericDataSafeClass::Get_Type_Size(int type)
{
	ds_assert(type >= 0);
	ds_assert(type < MAX_DATASAFE_TYPES);
	ds_assert(type < TypeListCount);
	return(TypeList[type].Size);
}





/***********************************************************************************************
 * GenericDataSafeClass::Security_Fault -- Security fault handler                              *
 *                                                                                             *
 *   __forceinline used to make sure there are multiple copies of this code. It's bloaty       *
 *   but it makes it harder to hack out (but only a little bit)                                *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/9/2001 2:20PM ST : Created                                                              *
 *=============================================================================================*/
__forceinline void GenericDataSafeClass::Security_Fault(void)
{
	WWDEBUG_SAY(("Data Safe:Security fault\n"));
	CRCErrors++;

	Say_Security_Fault();
	ds_assert(false);
}


/***********************************************************************************************
 * GenericDataSafeClass::Security_Check -- Look for security problems.                         *
 *                                                                                             *
 *   __forceinline used to make sure there are multiple copies of this code. It's bloaty       *
 *   but it makes it harder to hack out (but only a little bit)                                *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/9/2001 1:08PM ST : Created                                                              *
 *=============================================================================================*/
__forceinline void GenericDataSafeClass::Security_Check(void)
{
	ThreadLockClass locker;

	/*
	** Flag to prevent recursion.
	*/
	static bool _checking = false;

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
	** See if SimpleKey has been zeroed out.
	*/
	if (HandleKey - SimpleKey == HandleKey) {
		WWDEBUG_SAY(("Data Safe: Simple Key compromised!\n"));
		Security_Fault();
	}

	/*
	** See if HandleKey has been zeroed out.
	*/
	if (SimpleKey - HandleKey == SimpleKey) {
		WWDEBUG_SAY(("Data Safe: Handle Key compromised!\n"));
		Security_Fault();
	}

	/*
	** Use the checksum to verify the whole data safe.
	** Yes, the whole safe. A teeny bit slow maybe.
	**
	** Since we are going through the whole safe here, we might as well make a note of where slop
	** needs to be added or removed and count how many total slop entries we have.
	*/
	unsigned int time = TIMEGETTIME();
	if (time < SecurityCheckDelay || (time | SecurityCheckDelay) == 0 || (time - SecurityCheckDelay) > SECURITY_CHECK_TIME) {

#ifdef WWDEBUG
		SlopCount = 0;
		NumSecurityChecks++;
#endif	//WWDEBUG

		/*
		** If toooo much time has gone by then do it anyway.
		*/
		if (!_checking) {	// || (time - SecurityCheckDelay) > SECURITY_CHECK_TIME * 5) {

			_checking = true;
			//WWDEBUG_SAY(("Data Safe: Performing security check\n"));
			SecurityCheckDelay = time;
			unsigned int checkey = ~SimpleKey;

			/*
			** Loop through every list.
			*/
			for (int i=0 ; i<NumLists ; i++) {
				ds_assert(Safe[i] != NULL);
				if (Safe[i] != NULL && Safe[i]->EntryCount > 0) {

					/*
					** Dereference stuff - make sure the list makes sense.
					*/
					DataSafeEntryClass *entry_ptr = Safe[i]->SafeList;
					unsigned int *data = NULL;
					ds_assert(entry_ptr != NULL);
					int data_size = entry_ptr->Size;
					ds_assert((data_size & 3) == 0);
					data_size = data_size >> 2;

					if (entry_ptr) {

						/*
						** Go through each entry in the list.
						*/
						for (int j=0 ; j<Safe[i]->EntryCount ; j++) {
#ifdef WWDEBUG
							/*
							** Count slop entries for debug purposes.
							*/
							if (entry_ptr->IsSlop) {
								SlopCount++;
							}
#endif	//WWDEBUG
							/*
							** Add in the handle.
							*/
							checkey ^= entry_ptr->Handle;

							/*
							** Add in the data.
							*/
							data = (unsigned int *) (((char*)entry_ptr) + sizeof(*entry_ptr));
							for (int z=0 ; z<data_size ; z++) {
								checkey ^= *data++;
							}

							/*
							** Next entry.
							*/
							entry_ptr = entry_ptr->Next;
						}
					}
				}
			}

			if (checkey != Checksum) {
				WWDEBUG_SAY(("Data Safe: Incorrect checksum!\n"));
				Security_Fault();
			}

			/*
			** Check the values of the sentinels.
			*/
			if (NumLists) {
				int *val = NULL;
				if ((int)SentinelOne != 0) {
					bool got = DataSafeClass<int>::Get(SentinelOne, val);
					if (!got || val == NULL || *val != SENTINEL_ONE) {
						WWDEBUG_SAY(("Data Safe: Incorrect sentinel value!\n"));
						Security_Fault();
					}
				}
				val = NULL;
				if ((int)SentinelTwo != 0) {
					bool got = DataSafeClass<int>::Get(SentinelTwo, val);
					if (!got || val == NULL || *val != SENTINEL_TWO) {
						WWDEBUG_SAY(("Data Safe: Incorrect sentinel value!\n"));
						Security_Fault();
					}
				}
			}
#ifdef	WWDEBUG
			Print_Safe_Stats_To_Debug_Output();
#endif	//WWDEBUG
			_checking = false;
		}
	}
}









/*
**
**
**    DataSafeClass functions.
**
**
*/


/***********************************************************************************************
 * DataSafeClass::DataSafeClass -- Class constructor                                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Bogus pointer. Pointer that isn't referenced. It's only used for type info        *
 *           Number of 'slop' entries to create                                                *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/25/2001 1:51PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
DataSafeClass<T>::DataSafeClass(T*, int slopcount)
{
	ThreadLockClass locker;

	/*
	** Clear out the return list.
	*/
	memset(ReturnList, 0, sizeof(ReturnList));
	ReturnIndex = 0;

	/*
	** Get the type info for 'T'. Different types get stored in different places in the safe. Only similar types can be in the
	** same safe list.
	*/
	int data_size = sizeof(T);
	uintptr_t type_code = Get_Type_Code();

	/*
	** Get_Type_ID will return the Type ID for this type code (instruction pointer). It will create a new type if this one
	** doesn't already exist.
	*/
	int id = Get_Type_ID(type_code, data_size);
	Type = id;

	MinSlop = slopcount;

	/*
	** Add some slop entries here. These are just bogues entries than can be swapped in memory with real entries. The less
	** commonly added data types should be give more slop entries.
	*/
	for (int i=0 ; i<slopcount ; i++) {

		/*
		** We need to make sure the constructor gets called for this object type before it's added to the safe. But we must
		** also make sure that the destructor *isn't* called when we go out of scope here - it will be called explicitly
		** when the datasafe is destructed. By using _alloca, and doing an in-place new, the memory that the object sits in
		** will be freed when it goes out of scope but the destructor for 'T' won't be called.
		*/
		void *stackmem = _alloca(sizeof(T));
		T *slop_ptr = new (stackmem) T;
		Add_Entry(*slop_ptr, true);
	}
}




/***********************************************************************************************
 * DataSafeClass::~DataSafeClass -- Class destructor. Removes remaining slop entries.          *
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
 *   7/5/2001 11:38AM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
DataSafeClass<T>::~DataSafeClass(void)
{
	ThreadLockClass locker;

	/*
	** Remove all slop enries of our type from all lists. Note that this will delete non-slop entries too if any remain at this
	** time.
	*/
	for (int i=0 ; i<NumLists ; i++) {
		ds_assert(Safe[i] != NULL);
		if (Safe[i] != NULL && Safe[i]->EntryType == Type && Safe[i]->EntryCount > 0) {

			while (Safe[i]->EntryCount > 0) {

				/*
				** Delete the first entry in the list.
				*/
				int entries = Safe[i]->EntryCount;
				DataSafeHandleClass handle = Safe[i]->SafeList->Handle ^ SimpleKey;
				handle = handle ^ HandleKey;
				Delete_Entry(handle);

				/*
				** Safety check. If something goes wrong and we didn't delete the entry then we will never get out of this loop
				** so check for that case.
				*/
				ds_assert(entries == Safe[i]->EntryCount+1);
				if (entries != Safe[i]->EntryCount+1) {
					break;
				}
			}
		}
	}
	Shutdown();
}





/***********************************************************************************************
 * DataSafeClass::Get_Type_Code -- Get a unique code per type without using RTTI               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Type code                                                                         *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/2/2001 11:17AM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
uintptr_t DataSafeClass<T>::Get_Type_Code(void)
{
	/*
	** Make sure this function gets expanded multiple times for different types by referencing the type.
	*/
	volatile int data_size = sizeof(T);
	data_size = data_size;

	/*
	** Since we aren't using RTTI I need some other way of distinguishing types in the safe. Because it's templatised, this
	** code will get expanded once for each type it's used with. I will use the location in memory of the function to
	** uniquely identify each type. What a cunning plan.
	*/
	/*
	** FIXME: This cunning plan might break down when linking with `/OPT:ICF`.
	*/
	static uintptr_t instruction_pointer;
	instruction_pointer = 0;
	instruction_pointer = reinterpret_cast<uintptr_t>(DataSafeClass<T>::Get_Type_Code);

	ds_assert(instruction_pointer != 0);

	return(instruction_pointer);
}





/***********************************************************************************************
 * DataSafeClass::Get_Type_ID -- Get the id for the given type and size                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Type identifier code                                                              *
 *           Size of type                                                                      *
 *                                                                                             *
 * OUTPUT:   Type ID                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/27/2001 12:44PM ST : Created                                                            *
 *=============================================================================================*/
template <class T>
int DataSafeClass<T>::Get_Type_ID(unsigned int type_code, int size)
{
	int id = 0;

	/*
	** See if we already have an ID for this type code.
	*/
	for (int i=0 ; i<TypeListCount ; i++) {
		if (TypeList[i].TypeCode == type_code) {
			ds_assert(TypeList[i].Size == size);
			return(TypeList[i].ID);
		}
	}

	/*
	** Create a new type entry.
	*/
	ds_assert(TypeListCount < MAX_DATASAFE_TYPES);
	if (TypeListCount < MAX_DATASAFE_TYPES) {
		ds_assert(TypeList[TypeListCount].TypeCode == 0);
		ds_assert(TypeList[TypeListCount].Size == 0);
		TypeList[TypeListCount].TypeCode = type_code;
		TypeList[TypeListCount].ID = TypeListCount;
		TypeList[TypeListCount].Size = size;
		id = TypeListCount++;
	}

	return(id);
}




/***********************************************************************************************
 * DataSafeClass::Add_Entry -- Create a new data safe entry and init it to the value provided  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Value                                                                             *
 *                                                                                             *
 * OUTPUT:   Handle                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/19/2001 8:34PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
DataSafeHandleClass DataSafeClass<T>::Add_Entry(T &value, bool is_slop)
{
	ThreadLockClass locker;
	Security_Check();

	/*
	** Allocate memory for the new entry.
	*/
	DataSafeEntryClass *entry_ptr = (DataSafeEntryClass*) new char [sizeof(DataSafeEntryDataClass<T>)];
	ds_assert(entry_ptr);

	if (entry_ptr) {

		/*
		** Init the data safe entry to reasonable values.
		*/
		entry_ptr->Next = NULL;
		entry_ptr->Prev = NULL;
		entry_ptr->Size = sizeof(value);
		entry_ptr->IsSlop = is_slop;

		/*
		** Copy the data to the safe entry.
		*/
		DataSafeEntryDataClass<T>* data_ptr = (DataSafeEntryDataClass<T>*)  entry_ptr;
		Mem_Copy_Encrypt(& data_ptr->Data, &value, sizeof(value), true);

		/*
		** Find out which list we will be adding the entry to.
		*/
		int list = Get_Random_List_For_Insertion(Type);
		ds_assert(list >= 0 && list < MAX_DATASAFE_LISTS);
		ds_assert(Safe[list] != NULL);

		if (list != -1 && Safe[list] != NULL) {

			/*
			** Now we know the list, we can get the next index id for that list and build a handle for the entry.
			*/
			int index = Get_Handle_ID(list);
			ds_assert(index >= 0 && index < MAX_ENTRIES_PER_LIST);
			DataSafeHandleClass handle;
			handle.Handle.Part.List = list;
			handle.Handle.Part.ID = index;
			handle.Handle.Part.Type = Type;

			/*
			** We have a handle. Better encrypt it.
			*/
			DataSafeHandleClass encrypted_handle = handle ^ SimpleKey;
			Checksum = Checksum ^ encrypted_handle;

			/*
			** Store it in the entry so we can do a search and match to get this entry out again.
			*/
			entry_ptr->Handle = encrypted_handle;

			/*
			** Insert the entry into the chosen list.
			*/
			Random_Insertion(entry_ptr, list, Type, is_slop);

			/*
			** Maybe add slop here if there is only one entry in the list.
			** It will get removed later as more entries are added.
			*/
			if (Safe[list]->EntryCount == 1 && !is_slop) {
				for (int i=0 ; i<MinSlop ; i++) {
					entry_ptr = (DataSafeEntryClass*) new char [sizeof(DataSafeEntryDataClass<T>)];
					ds_assert(entry_ptr);

					if (entry_ptr) {
						entry_ptr->Next = NULL;
						entry_ptr->Prev = NULL;
						entry_ptr->Size = sizeof(T);
						entry_ptr->IsSlop = true;

						/*
						** Encrypt the garbage data in the uninitialized memory we just allocated for the
						** object. It has to be encrypted to keep the CRC right.
						*/
						Encrypt(&((DataSafeEntryDataClass<T>*)entry_ptr)->Data, sizeof(T), SimpleKey, true);

						/*
						** Get a handle ID. Not really needed since we won't be returning a handle but it
						** keeps the ID usage list neat. Fill in the rest of the handle to make it valid.
						*/
						DataSafeHandleClass slop_handle;
						slop_handle.Handle.Part.ID = Get_Handle_ID(list);
						slop_handle.Handle.Part.List = list;
						slop_handle.Handle.Part.Type = Type;
						DataSafeHandleClass encrypted_slop_handle = slop_handle ^ SimpleKey;
						Checksum = Checksum ^ encrypted_slop_handle;
						entry_ptr->Handle = encrypted_slop_handle;

						/*
						** Insert the slop entry.
						*/
						Random_Insertion(entry_ptr, list, Type, true);
					}
				}
				ds_assert(Safe[list]->SlopCount == MinSlop);
				ds_assert(Safe[list]->EntryCount == MinSlop + 1);
			}

			/*
			** Maybe remove some slop if the list is filling up.
			*/
			if (Safe[list]->SlopCount != 0 && Safe[list]->EntryCount > Safe[list]->SlopCount * 2 && !is_slop) {

				/*
				** Lots of entries here. Remove all slop by building a list of slop entries then
				** removing them one by one.
				*/
				entry_ptr = Safe[list]->SafeList;
				ds_assert(entry_ptr != NULL);
				DataSafeHandleClass removal_list[MAX_ENTRIES_PER_LIST];
				int removal_count = 0;

				if (entry_ptr) {

					/*
					** Go through each entry in the list.
					*/
					for (int j=0 ; j<Safe[list]->EntryCount ; j++) {

						/*
						** If this is a slop entry then mark it for removal.
						*/
						if (entry_ptr->IsSlop) {
							DataSafeHandleClass decode_handle = entry_ptr->Handle ^ SimpleKey;
							decode_handle = decode_handle ^ HandleKey;
							removal_list[removal_count++] = decode_handle;
						}

						/*
						** Next entry.
						*/
						entry_ptr = entry_ptr->Next;
					}
				}

				/*
				** Remove all marked entries. Delete_Entry will fix up the lists slop count.
				*/
				for (int j=0 ; j<removal_count ; j++) {
					Delete_Entry(removal_list[j]);
				}
				ds_assert(Safe[list]->SlopCount == 0);
			}

			/*
			** Return the handle, encrypted using the handle key.
			*/
			handle.Handle.Whole.WholeHandle ^= HandleKey;
			return(handle);
		}
	}

	/*
	** Allocation error.
	*/
	return((DataSafeHandleClass)-1);

}




/***********************************************************************************************
 * DataSafeClass::Delete_Entry -- Delete an entry from the data safe                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Handle to entry                                                                   *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/25/2001 1:32PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
void DataSafeClass<T>::Delete_Entry(DataSafeHandleClass handle)
{
	/*
	** Locals.
	*/
	ThreadLockClass locker;
	int list;
	int id;

	/*
	** Asserts.
	*/
	ds_assert(handle.Is_Valid());

	/*
	** Check safe integrity.
	*/
	Security_Check();

	/*
	** The handles we give out are encrypted so we need to decrypt temporarily to extract the list number.
	*/
	DataSafeHandleClass new_handle = handle ^ HandleKey;
	list = new_handle.Handle.Part.List;
	id = new_handle.Handle.Part.ID;

	/*
	** Check that list number.
	*/
	ds_assert(list >= 0);
	ds_assert(list < NumLists);
	ds_assert(Safe[list] != NULL);

	/*
	** Get a pointer to the actual entry in the safe list.
	*/
	DataSafeEntryClass *entry_ptr = Get_Entry(handle);
	ds_assert(entry_ptr != NULL);

	if (entry_ptr != NULL) {

		/*
		** Apply the current data key to the handle to fix up the checksum.
		*/
		DataSafeHandleClass match_handle = new_handle ^ SimpleKey;
		Checksum ^= match_handle;

		/*
		** We found the entry for this handle. We need to fix up the list pointers and delete the memory.
		** Since we originally memcpy'ed this object into place we will have to call the destructor explicitly. The explicit
		** destructor call will have no effect if the object doesn't have a destructor defined.
		**
		** We need to decrypt the object in place before calling the desturctor otherwise it will have a hard time acting on
		** it's own data. This has the bonus of fixing up the checksum too.
		*/
		DataSafeEntryDataClass<T> *whole_entry_ptr = (DataSafeEntryDataClass<T>*) entry_ptr;
		Decrypt(&whole_entry_ptr->Data, Get_Type_Size(Safe[list]->EntryType), SimpleKey);
		if (!whole_entry_ptr->IsSlop) {
			whole_entry_ptr->Data.T::~T();
		}
		Remove_From_List(list, entry_ptr);
		Free_Handle_ID(list, id);
		delete [] (char*) entry_ptr;
		return;
	}

	/*
	** No match found.
	*/
	WWDEBUG_SAY(("Datasafe: No item found for handle %08x\n", (int)handle));
}




/***********************************************************************************************
 * DataSafeClass::Get -- Get a copy of an item in the data safe.                               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Handle to data safe entry                                                         *
 *           (out) Pointer to copy of object                                                   *
 *                                                                                             *
 * OUTPUT:   True if object retrieved OK                                                       *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/2/2001 11:07AM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
bool DataSafeClass<T>::Get(DataSafeHandleClass handle, T* &item)
{
	ThreadLockClass locker;

	/*
	** Get a pointer to the entry.
	*/
	DataSafeEntryClass *entry_ptr = Get_Entry(handle);

	if (entry_ptr == NULL) {
		return(false);
	}

	DataSafeEntryDataClass<T> *data_entry_ptr = (DataSafeEntryDataClass<T>*) entry_ptr;

	/*
	** Copy the object from the data safe and decrypt it in one pass. Don't adjust the checksum as the object is staying
	** the same in the safe.
	*/
	Mem_Copy_Decrypt(&ReturnList[ReturnIndex][0], &data_entry_ptr->Data, sizeof(T), false);

	/*
	** Return the object.
	*/
	item = (T*)(&ReturnList[ReturnIndex][0]);

	/*
	** Fix up the return list index.
	*/
	ReturnIndex++;
	if (ReturnIndex >= MAX_OBJECT_COPIES) {
		ReturnIndex = 0;
	}

	/*
	** Move stuff around.
	*/
	Shuffle();

	/*
	** Check safe integrity.
	*/
	Security_Check();

	return(true);
}




/***********************************************************************************************
 * DataSafeClass::Set -- Set the value of an entry in the data safe                            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Handle to data in the safe                                                        *
 *           New value to set                                                                  *
 *                                                                                             *
 * OUTPUT:   True if handle was valid                                                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:11PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
bool DataSafeClass<T>::Set(DataSafeHandleClass handle, T *data)
{
	ThreadLockClass locker;

	/*
	** Get a pointer to the entry.
	*/
	DataSafeEntryClass *entry_ptr = Get_Entry(handle);

	if (entry_ptr == NULL) {
		return(false);
	}

	DataSafeEntryDataClass<T> *data_entry_ptr = (DataSafeEntryDataClass<T>*) entry_ptr;

	/*
	** Decrypt the old entry just to fix up the checksum.
	*/
	Decrypt(&data_entry_ptr->Data, sizeof(T), SimpleKey, true);

	/*
	** Copy the object into the data safe and encrypt it in one pass.
	*/
	Mem_Copy_Encrypt(&data_entry_ptr->Data, data, sizeof(T), true);

	/*
	** Move stuff around.
	*/
	Shuffle();

	/*
	** Check safe integrity.
	*/
	Security_Check();

	return(true);
}







/*
**
**
** 	SafeDataClass functions.
**
**
*/


/***********************************************************************************************
 * SafeDataClass::SafeDataClass -- Class constructor. Adds the data to the data safe           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Initial value to store in the safe                                                *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 11:50AM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline SafeDataClass<T>::SafeDataClass(T data)
{
	Handle = DataSafeClass<T>::Add_Entry(data);
#ifdef WWDEBUG
	DebugData = data;
#endif //WWDEBUG
}


/***********************************************************************************************
 * SafeDataClass::SafeDataClass -- Class constructor. Adds the fetches a safe handle           *
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
 *   7/3/2001 11:50AM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
SafeDataClass<T>::SafeDataClass(void)
{
	/*
	** The data here isn't initialised since no default value was provided. I have to provide *some* sort of
	** init though as the bloody compiler won't let me turn off the 'local variable 'data' used without having been initialized'
	** message. Grrrr.
	**
	** Allocating the memory on the stack then doing an in place new seems to mollify the compiler so we don't get the warning
	** and it does much the same thing as just declaring 'T data'. It also has the benefit that the destructor won't be called
	** for the slop object when we exit this function but the memory will be freed.
	*/
	void *stackmem = _alloca(sizeof(T));
	T *data = new (stackmem) T;

	/*
	** Add the entry to the data safe and note the handle.
	*/
	Handle = DataSafeClass<T>::Add_Entry(*data);
#ifdef WWDEBUG
	DebugData = *data;
#endif //WWDEBUG

}


/***********************************************************************************************
 * SafeDataClass::~SafeDataClass -- Class desturctor. Removes the data from the data safe      *
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
 *   7/3/2001 11:51AM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline SafeDataClass<T>::~SafeDataClass(void)
{
	if (Handle.Is_Valid()) {
		DataSafeClass<T>::Delete_Entry(Handle);
	}
}




/***********************************************************************************************
 * SafeDataClass::operator = - assignment operator.                                            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to assign                                                                    *
 *                                                                                             *
 * OUTPUT:   Data assigned                                                                     *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator = (T const &data)
{
	/*
	** If we have a valid handle, then set the data into the data safe.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Set(Handle, (T*) &data);
		ds_assert(ok);
#ifdef WWDEBUG
		DebugData = data;
#endif //WWDEBUG
	}

	return((T&)data);
}




/***********************************************************************************************
 * SafeDataClass::operator = - assignment operator for assigning from other SafeDataClass's    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to assign                                                                    *
 *                                                                                             *
 * OUTPUT:   Data assigned                                                                     *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator = (SafeDataClass<T> &safedata)
{
	/*
	** Locals.
	*/
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safedata.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safedata.Get_Handle()));

	/*
	** If we have a valid handle, then get the data for this handle from the data safe.
	*/
	if (Handle.Is_Valid()) {
		/*
		** Get the value for the other safe data class
		** and set it into the safe entry with our handle.
		*/
		other_value = safedata.Get_Ptr();
		ds_assert(other_value != NULL);

		if (other_value) {
			[[maybe_unused]] bool ok = DataSafeClass<T>::Set(Handle, (T*) other_value);
			ds_assert(ok);
#ifdef WWDEBUG
			DebugData = *other_value;
#endif //WWDEBUG
		} else {
			/*
			** Bad error case, have to return something so use the value we already have.
			*/
			return((T&)*other_value);
		}
	}

	return((T&)*other_value);
}






/***********************************************************************************************
 * SafeDataClass::operator == - Equality check operator.                                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to compare against                                                           *
 *                                                                                             *
 * OUTPUT:   true if equal                                                                     *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline bool SafeDataClass<T>::operator == (T const &data)
{
	T *data_ptr = NULL;

	/*
	** If we have a valid handle, then check the value against the supplied data.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			return(*data_ptr == data);
		}
	}

	return(false);
}






/***********************************************************************************************
 * SafeDataClass::operator == - Equality check operator.                                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Safe Data to compare against                                                      *
 *                                                                                             *
 * OUTPUT:   true if equal                                                                     *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline bool SafeDataClass<T>::operator == (SafeDataClass<T> &safedata)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safedata.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safedata.Get_Handle()));

	/*
	** If we have a valid handle, then check the value against the supplied data.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {

			/*
			** Looks like we have the value for this safe data class. Get the value for the other one.
			*/
			other_value = safedata.Get_Ptr();
			ds_assert(other_value != NULL);

			if (other_value) {

				/*
				** Check them against each other,
				*/
				return(*data_ptr == *other_value);
			}
		}
	}

	return(false);
}






/***********************************************************************************************
 * SafeDataClass::operator != - Inequality check operator.                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to compare against                                                           *
 *                                                                                             *
 * OUTPUT:   true if not equal                                                                 *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline bool SafeDataClass<T>::operator != (T const &data)
{
	T *data_ptr = NULL;

	/*
	** If we have a valid handle, then check the value against the supplied data.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			return(*data_ptr != data);
		}
	}

	return(true);
}




/***********************************************************************************************
 * SafeDataClass::operator != - Inequality check operator.                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to compare against                                                           *
 *                                                                                             *
 * OUTPUT:   true if not equal                                                                 *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline bool SafeDataClass<T>::operator != (SafeDataClass<T> &safedata)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safedata.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safedata.Get_Handle()));

	/*
	** If we have a valid handle, then check the value against the supplied data.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {

			/*
			** Looks like we have the value for this safe data class. Get the value for the other one.
			*/
			other_value = safedata.Get_Ptr();
			ds_assert(other_value != NULL);

			if (other_value) {

				/*
				** Check them against each other,
				*/
				return(*data_ptr != *other_value);
			}
		}
	}

	return(false);
}






/***********************************************************************************************
 * SafeDataClass::operator > - > operator.                                                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to compare against                                                           *
 *                                                                                             *
 * OUTPUT:   true if greater than                                                              *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline bool SafeDataClass<T>::operator > (T const &data)
{
	T *data_ptr = NULL;

	/*
	** If we have a valid handle, then check the value against the supplied data.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			return(*data_ptr > data);
		}
	}

	return(false);
}






/***********************************************************************************************
 * SafeDataClass::operator > - > operator.                                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Safe Data to compare against                                                      *
 *                                                                                             *
 * OUTPUT:   true if greater than                                                              *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline bool SafeDataClass<T>::operator > (SafeDataClass<T> &safedata)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safedata.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safedata.Get_Handle()));

	/*
	** If we have a valid handle, then check the value against the supplied data.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {

			/*
			** Looks like we have the value for this safe data class. Get the value for the other one.
			*/
			other_value = safedata.Get_Ptr();
			ds_assert(other_value != NULL);

			if (other_value) {

				/*
				** Check them against each other,
				*/
				return(*data_ptr > *other_value);
			}
		}
	}

	return(false);
}








/***********************************************************************************************
 * SafeDataClass::operator >= - >= operator.                                                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to compare against                                                           *
 *                                                                                             *
 * OUTPUT:   true if greater than or equal                                                     *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline bool SafeDataClass<T>::operator >= (T const &data)
{
	T *data_ptr = NULL;

	/*
	** If we have a valid handle, then check the value against the supplied data.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			return(*data_ptr >= data);
		}
	}

	return(false);
}






/***********************************************************************************************
 * SafeDataClass::operator >= - >= operator.                                                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Safe Data to compare against                                                      *
 *                                                                                             *
 * OUTPUT:   true if greater than or equal                                                     *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline bool SafeDataClass<T>::operator >= (SafeDataClass<T> &safedata)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safedata.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safedata.Get_Handle()));

	/*
	** If we have a valid handle, then check the value against the supplied data.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {

			/*
			** Looks like we have the value for this safe data class. Get the value for the other one.
			*/
			other_value = safedata.Get_Ptr();
			ds_assert(other_value != NULL);

			if (other_value) {

				/*
				** Check them against each other,
				*/
				return(*data_ptr >= *other_value);
			}
		}
	}

	return(false);
}







/***********************************************************************************************
 * SafeDataClass::operator < - < operator.                                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to compare against                                                           *
 *                                                                                             *
 * OUTPUT:   true if less than                                                                 *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline bool SafeDataClass<T>::operator < (T const &data)
{
	T *data_ptr = NULL;

	/*
	** If we have a valid handle, then check the value against the supplied data.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			return(*data_ptr < data);
		}
	}

	return(false);
}






/***********************************************************************************************
 * SafeDataClass::operator < - < operator.                                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Safe Data to compare against                                                      *
 *                                                                                             *
 * OUTPUT:   true if less than                                                                 *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline bool SafeDataClass<T>::operator < (SafeDataClass<T> &safedata)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safedata.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safedata.Get_Handle()));

	/*
	** If we have a valid handle, then check the value against the supplied data.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {

			/*
			** Looks like we have the value for this safe data class. Get the value for the other one.
			*/
			other_value = safedata.Get_Ptr();
			ds_assert(other_value != NULL);

			if (other_value) {

				/*
				** Check them against each other,
				*/
				return(*data_ptr < *other_value);
			}
		}
	}

	return(false);
}










/***********************************************************************************************
 * SafeDataClass::operator <= - <= operator.                                                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to compare against                                                           *
 *                                                                                             *
 * OUTPUT:   true if less than or equal                                                        *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline bool SafeDataClass<T>::operator <= (T const &data)
{
	T *data_ptr = NULL;

	/*
	** If we have a valid handle, then check the value against the supplied data.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			return(*data_ptr <= data);
		}
	}

	return(false);
}






/***********************************************************************************************
 * SafeDataClass::operator <= - <= operator.                                                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Safe Data to compare against                                                      *
 *                                                                                             *
 * OUTPUT:   true if less than or equal                                                        *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline bool SafeDataClass<T>::operator <= (SafeDataClass<T> &safedata)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safedata.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safedata.Get_Handle()));

	/*
	** If we have a valid handle, then check the value against the supplied data.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {

			/*
			** Looks like we have the value for this safe data class. Get the value for the other one.
			*/
			other_value = safedata.Get_Ptr();
			ds_assert(other_value != NULL);

			if (other_value) {

				/*
				** Check them against each other,
				*/
				return(*data_ptr <= *other_value);
			}
		}
	}

	return(false);
}






/***********************************************************************************************
 * SafeDataClass::operator + - + operator.                                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to add                                                                       *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator + (T const &value)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());

	/*
	** If we have a valid handle, then get the current value and apply the change
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			*data_ptr = *data_ptr + value;
			return((T&)*data_ptr);
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}



/***********************************************************************************************
 * SafeDataClass::operator + - + operator                                                      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to add                                                                       *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator + (SafeDataClass<T> &safevalue)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safevalue.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safevalue.Get_Handle()));

	/*
	** If we have a valid handle, then get the data for this handle from the data safe.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			/*
			** Looks like we have the value for this safe data class. Get the value for the other one.
			** and do the math.
			*/
			other_value = safevalue.Get_Ptr();
			ds_assert(other_value != NULL);

			if (other_value) {
				*data_ptr = *data_ptr + *other_value;
				return((T&)*data_ptr);
			} else {
				/*
				** Bad error case, have to return something so use the value we already have.
				*/
				return((T&)*data_ptr);
			}
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}









/***********************************************************************************************
 * SafeDataClass::operator += - += operator.                                                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to add                                                                       *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator += (T const &value)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());

	/*
	** If we have a valid handle, then get the current value and apply the change
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			*data_ptr = *data_ptr + value;

			ok = DataSafeClass<T>::Set(Handle, data_ptr);
			ds_assert(ok);
#ifdef WWDEBUG
			DebugData = *data_ptr;
#endif //WWDEBUG
			return((T&)*data_ptr);
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}



/***********************************************************************************************
 * SafeDataClass::operator += - += operator                                                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to add                                                                       *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator += (SafeDataClass<T> &safevalue)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safevalue.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safevalue.Get_Handle()));

	/*
	** If we have a valid handle, then get the data for this handle from the data safe.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			/*
			** Looks like we have the value for this safe data class. Get the value for the other one.
			** and do the math.
			*/
			other_value = safevalue.Get_Ptr();
			ds_assert(other_value != NULL);

			if (other_value) {
				*data_ptr = *data_ptr + *other_value;
#ifdef WWDEBUG
				ok =
#endif //WWDEBUG
					DataSafeClass<T>::Set(Handle, data_ptr);
				ds_assert(ok);
#ifdef WWDEBUG
				DebugData = *data_ptr;
#endif //WWDEBUG
				return((T&)*data_ptr);
			} else {
				/*
				** Bad error case, have to return something so use the value we already have.
				*/
				return((T&)*data_ptr);
			}
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}







/***********************************************************************************************
 * SafeDataClass::operator - - - operator.                                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to subtract                                                                  *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator - (T const &value)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());

	/*
	** If we have a valid handle, then get the current value and apply the change
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			*data_ptr = *data_ptr - value;
			return((T&)*data_ptr);
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}



/***********************************************************************************************
 * SafeDataClass::operator - - - operator                                                      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to subtract                                                                  *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator - (SafeDataClass<T> &safevalue)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safevalue.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safevalue.Get_Handle()));

	/*
	** If we have a valid handle, then get the data for this handle from the data safe.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			/*
			** Looks like we have the value for this safe data class. Get the value for the other one.
			** and do the math.
			*/
			other_value = safevalue.Get_Ptr();
			ds_assert(other_value != NULL);

			if (other_value) {
				*data_ptr = *data_ptr - *other_value;
				return((T&)*data_ptr);
			} else {
				/*
				** Bad error case, have to return something so use the value we already have.
				*/
				return((T&)*data_ptr);
			}
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}









/***********************************************************************************************
 * SafeDataClass::operator -= - -= operator.                                                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to subtract                                                                  *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator -= (T const &value)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());

	/*
	** If we have a valid handle, then get the current value and apply the change
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			*data_ptr = *data_ptr - value;

			ok = DataSafeClass<T>::Set(Handle, data_ptr);
			ds_assert(ok);
#ifdef WWDEBUG
			DebugData = *data_ptr;
#endif //WWDEBUG
			return((T&)*data_ptr);
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}



/***********************************************************************************************
 * SafeDataClass::operator -= - -= operator                                                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to subtract                                                                  *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator -= (SafeDataClass<T> &safevalue)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safevalue.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safevalue.Get_Handle()));

	/*
	** If we have a valid handle, then get the data for this handle from the data safe.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			/*
			** Looks like we have the value for this safe data class. Get the value for the other one.
			** and do the math.
			*/
			other_value = safevalue.Get_Ptr();
			ds_assert(other_value != NULL);

			if (other_value) {
				*data_ptr = *data_ptr - *other_value;
#ifdef WWDEBUG
				ok =
#endif //WWDEBUG
					DataSafeClass<T>::Set(Handle, data_ptr);
				ds_assert(ok);
#ifdef WWDEBUG
				DebugData = *data_ptr;
#endif //WWDEBUG
				return((T&)*data_ptr);
			} else {
				/*
				** Bad error case, have to return something so use the value we already have.
				*/
				return((T&)*data_ptr);
			}
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}











/***********************************************************************************************
 * SafeDataClass::operator * - * operator.                                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to multiply by                                                               *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator * (T const &value)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());

	/*
	** If we have a valid handle, then get the current value and apply the change
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			*data_ptr = *data_ptr * value;
			return((T&)*data_ptr);
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}



/***********************************************************************************************
 * SafeDataClass::operator * - * operator                                                      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to multiply by                                                               *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator * (SafeDataClass<T> &safevalue)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safevalue.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safevalue.Get_Handle()));

	/*
	** If we have a valid handle, then get the data for this handle from the data safe.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			/*
			** Looks like we have the value for this safe data class. Get the value for the other one.
			** and do the math.
			*/
			other_value = safevalue.Get_Ptr();
			ds_assert(other_value != NULL);

			if (other_value) {
				*data_ptr = *data_ptr * *other_value;
				return((T&)*data_ptr);
			} else {
				/*
				** Bad error case, have to return something so use the value we already have.
				*/
				return((T&)*data_ptr);
			}
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}









/***********************************************************************************************
 * SafeDataClass::operator *= - *= operator.                                                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to multiply by                                                               *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator *= (T const &value)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());

	/*
	** If we have a valid handle, then get the current value and apply the change
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			*data_ptr = *data_ptr * value;

			ok = DataSafeClass<T>::Set(Handle, data_ptr);
			ds_assert(ok);
#ifdef WWDEBUG
			DebugData = *data_ptr;
#endif //WWDEBUG
			return((T&)*data_ptr);
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}



/***********************************************************************************************
 * SafeDataClass::operator *= - *= operator                                                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to multiply by                                                               *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator *= (SafeDataClass<T> &safevalue)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safevalue.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safevalue.Get_Handle()));

	/*
	** If we have a valid handle, then get the data for this handle from the data safe.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			/*
			** Looks like we have the value for this safe data class. Get the value for the other one.
			** and do the math.
			*/
			other_value = safevalue.Get_Ptr();
			ds_assert(other_value != NULL);

			if (other_value) {
				*data_ptr = *data_ptr * *other_value;
#ifdef WWDEBUG
				ok =
#endif //WWDEBUG
					DataSafeClass<T>::Set(Handle, data_ptr);
				ds_assert(ok);
#ifdef WWDEBUG
				DebugData = *data_ptr;
#endif //WWDEBUG
				return((T&)*data_ptr);
			} else {
				/*
				** Bad error case, have to return something so use the value we already have.
				*/
				return((T&)*data_ptr);
			}
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}





/***********************************************************************************************
 * SafeDataClass::operator / - / operator.                                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to divide by                                                                 *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator / (T const &value)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(value != 0);

	/*
	** If we have a valid handle, then get the current value and apply the change
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			*data_ptr = *data_ptr / value;
			return((T&)*data_ptr);
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}



/***********************************************************************************************
 * SafeDataClass::operator / - / operator                                                      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to divide by                                                                 *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator / (SafeDataClass<T> &safevalue)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safevalue.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safevalue.Get_Handle()));

	/*
	** If we have a valid handle, then get the data for this handle from the data safe.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			/*
			** Looks like we have the value for this safe data class. Get the value for the other one.
			** and do the math.
			*/
			other_value = safevalue.Get_Ptr();
			ds_assert(other_value != NULL);

			if (other_value) {
				ds_assert(*other_value != 0);
				*data_ptr = *data_ptr / *other_value;
				return((T&)*data_ptr);
			} else {
				/*
				** Bad error case, have to return something so use the value we already have.
				*/
				return((T&)*data_ptr);
			}
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}









/***********************************************************************************************
 * SafeDataClass::operator /= - /= operator.                                                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to divide by                                                                 *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator /= (T const &value)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(value != 0);

	/*
	** If we have a valid handle, then get the current value and apply the change
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			*data_ptr = *data_ptr / value;

			ok = DataSafeClass<T>::Set(Handle, data_ptr);
			ds_assert(ok);
#ifdef WWDEBUG
			DebugData = *data_ptr;
#endif //WWDEBUG
			return((T&)*data_ptr);
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}



/***********************************************************************************************
 * SafeDataClass::operator /= - /= operator                                                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Data to divide by                                                                 *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator /= (SafeDataClass<T> &safevalue)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;
	T *other_value = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());
	ds_assert(safevalue.Get_Handle().Is_Valid());
	ds_assert(GenericDataSafeClass::Get_Entry_Type(Handle) == GenericDataSafeClass::Get_Entry_Type(safevalue.Get_Handle()));

	/*
	** If we have a valid handle, then get the data for this handle from the data safe.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			/*
			** Looks like we have the value for this safe data class. Get the value for the other one.
			** and do the math.
			*/
			other_value = safevalue.Get_Ptr();
			ds_assert(other_value != NULL);

			if (other_value) {
				ds_assert(*other_value != 0);
				*data_ptr = *data_ptr / *other_value;
				ok = DataSafeClass<T>::Set(Handle, data_ptr);
				ds_assert(ok);
#ifdef WWDEBUG
				DebugData = *data_ptr;
#endif //WWDEBUG
				return((T&)*data_ptr);
			} else {
				/*
				** Bad error case, have to return something so use the value we already have.
				*/
				return((T&)*data_ptr);
			}
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}



/***********************************************************************************************
 * SafeDataClass::operator ++ - prefix increment operator.                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator ++ (void)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());

	/*
	** If we have a valid handle, then get the current value and apply the change
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			(*data_ptr)++;

			ok = DataSafeClass<T>::Set(Handle, data_ptr);
			ds_assert(ok);
#ifdef WWDEBUG
			DebugData = *data_ptr;
#endif //WWDEBUG
			return((T&)*data_ptr);
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}





/***********************************************************************************************
 * SafeDataClass::operator -- - prefix decrement operator.                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Result                                                                            *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator -- (void)
{
	/*
	** Locals.
	*/
	T *data_ptr = NULL;

	/*
	** Asserts.
	*/
	ds_assert(Handle.Is_Valid());

	/*
	** If we have a valid handle, then get the current value and apply the change
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		ds_assert(data_ptr);

		if (data_ptr) {
			(*data_ptr)--;

			ok = DataSafeClass<T>::Set(Handle, data_ptr);
			ds_assert(ok);
#ifdef WWDEBUG
			DebugData = *data_ptr;
#endif //WWDEBUG
			return((T&)*data_ptr);
		}
	}

	/*
	** Error case, have to return something.
	*/
	return((T&)ErrorVal[0]);
}




/***********************************************************************************************
 * SafeDataClass::operator ++ - postfix ++ operator                                            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    int (not used, denotes postfix operator)                                          *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/23/2001 3:54PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator ++ (int)
{
	/*
	** Just call the prefix version of the operator.
	*/
	return(this->operator ++ ());
}



/***********************************************************************************************
 * SafeDataClass::operator -- - postfix -- operator                                            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    int (not used, denotes postfix operator)                                          *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/23/2001 3:54PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator -- (int)
{
	/*
	** Just call the prefix version of the operator.
	*/
	return(this->operator -- ());
}








/***********************************************************************************************
 * SafeDataClass::operator () - Conversion operator used for data retrieval                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Copy of data in safe                                                              *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T &SafeDataClass<T>::operator () (void) const
{
	T *data_ptr = NULL;

	/*
	** If the handle we have is valid then use it to get a pointer to a temporary copy of the data safe contents for this
	** handle.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		if (data_ptr) {
			return(*data_ptr);
		}
	}

	/*
	** Error case. Need to return some valid value.
	*/
	static T oh_dear;
	return(oh_dear);
}






/***********************************************************************************************
 * SafeDataClass::operator int -- Return the data for this class as an int                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Data cast to int                                                                  *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/6/2001 11:47AM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline SafeDataClass<T>::operator int (void) const
{
	ds_assert(sizeof(T) == sizeof(int));

	T *data_ptr = NULL;

#ifdef WWDEBUG
	/*
	** Check that T is safe to return as an int
	*/
	T x = 0;
	[[maybe_unused]] int y = (T)x;
	ds_assert(x == y);
#endif	//WWDEBUG

	/*
	** If the handle we have is valid then use it to get a pointer to a temporary copy of the data safe contents for this
	** handle.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		if (data_ptr) {
			return(*((int*)data_ptr));
		}
	}

	/*
	** Error case. Need to return some valid value.
	*/
	static int oh_dear;
	return(oh_dear);
}



/***********************************************************************************************
 * SafeDataClass::operator unsigned int -- Return the data for this class as an unsigned int   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Data cast to unsigned int                                                         *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/6/2001 11:47AM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline SafeDataClass<T>::operator unsigned int (void) const
{
	ds_assert(sizeof(T) == sizeof(unsigned int));

	T *data_ptr = NULL;

#ifdef WWDEBUG
	/*
	** Check that T is safe to return as an unsigned int
	*/
	T x = 0;
	[[maybe_unused]] unsigned int y = (T)x;
	ds_assert(x == y);
#endif	//WWDEBUG

	/*
	** If the handle we have is valid then use it to get a pointer to a temporary copy of the data safe contents for this
	** handle.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		if (data_ptr) {
			return(*((unsigned int*)data_ptr));
		}
	}

	/*
	** Error case. Need to return some valid value.
	*/
	static unsigned int oh_dear;
	return(oh_dear);
}



/***********************************************************************************************
 * SafeDataClass::operator int -- Return the data for this class as a float                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Data cast to float                                                                *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/6/2001 11:47AM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline SafeDataClass<T>::operator float (void) const
{
	ds_assert(sizeof(T) == sizeof(float));

	T *data_ptr = NULL;

#ifdef WWDEBUG
	/*
	** Check that T is safe to return as a float
	*/
	T x = 0;
	[[maybe_unused]] float y = (T)x;
	ds_assert(x == y);
#endif	//WWDEBUG

	/*
	** If the handle we have is valid then use it to get a pointer to a temporary copy of the data safe contents for this
	** handle.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		if (data_ptr) {
			return(*((float*)data_ptr));
		}
	}

	/*
	** Error case. Need to return some valid value.
	*/
	static float oh_dear;
	return(oh_dear);
}



/***********************************************************************************************
 * SafeDataClass::operator int -- Return the data for this class as a double                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Data cast to double                                                               *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/6/2001 11:47AM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline SafeDataClass<T>::operator double (void) const
{
	ds_assert(sizeof(T) == sizeof(double));

	T *data_ptr = NULL;

#ifdef WWDEBUG
	/*
	** Check that T is safe to return as a double
	*/
	T x = 0;
	[[maybe_unused]] double y = (T)x;
	ds_assert(x == y);
#endif	//WWDEBUG

	/*
	** If the handle we have is valid then use it to get a pointer to a temporary copy of the data safe contents for this
	** handle.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		if (data_ptr) {
			return(*((double*)data_ptr));
		}
	}

	/*
	** Error case. Need to return some valid value.
	*/
	static double oh_dear;
	return(oh_dear);
}









/***********************************************************************************************
 * SafeDataClass::Get_Ptr - Get a pointer to a decrypted version of a class in the data safe   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Ptr to copy of data in safe                                                       *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001 12:05PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline T *SafeDataClass<T>::Get_Ptr(void) const
{
	T *data_ptr = NULL;

	/*
	** If the handle we have is valid then use it to get a pointer to a temporary copy of the data safe contents for this
	** handle.
	*/
	if (Handle.Is_Valid()) {
		[[maybe_unused]] bool ok = DataSafeClass<T>::Get(Handle, data_ptr);
		ds_assert(ok);
		if (data_ptr) {
			return(data_ptr);
		}
	}

	/*
	** Error case.
	*/
	ds_assert(false);
	return(NULL);
}





/***********************************************************************************************
 * SafeDataClass::Commit -- Write decrypted data safe data back into the safe                  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Ptr to data (must be from prior return list)                                      *
 *                                                                                             *
 * OUTPUT:   True if commited back into the safe                                               *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/13/2001 2:28PM ST : Created                                                             *
 *=============================================================================================*/
template <class T>
inline bool SafeDataClass<T>::Commit(T *data_ptr) const
{
	ds_assert(Handle.Is_Valid());
	ds_assert(DataSafeClass<T>::In_Return_List(data_ptr));

	/*
	** If the handle is valid, and the data pointer is one of our temporary return buffers
	** then copy the data back into the safe.
	*/
	if (Handle.Is_Valid()) {

		/*
		** Verify it's one of our temp data pointers.
		*/
		if (DataSafeClass<T>::In_Return_List(data_ptr)) {
#ifdef WWDEBUG
			DebugData = *data_ptr;
#endif //WWDEBUG
			return(DataSafeClass<T>::Set(Handle, data_ptr));
		}
	}
	return(false);
}


#endif //_DATASAFE_H




