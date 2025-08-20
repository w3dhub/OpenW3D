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
 *                 Project Name : Commando / G                                                 * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Combat/reflist.h         $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 2/06/01 2:33p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 13                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	REFLIST_H
#define	REFLIST_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	WWDEBUG_H
	#include "wwdebug.h"
#endif

#ifndef	CHUNKIO_H
	#include "chunkio.h"
#endif

#ifndef	SAVELOAD_H
	#include "saveload.h"
#endif

#ifndef	PERSIST_H
	#include "persist.h"
#endif

/*
** Forward declarations
*/
class ScriptableGameObj;
class ReferencerClass;

/*
** Advance notice of referencer
*/
//template	<class T> class ReferencerClass;

/*
** A object class that can be referenced
*/
template	<class T>
class ReferenceableClass {

	public:
		friend class ReferencerClass;
		ReferenceableClass( T *reference_data ) : ReferenceData( reference_data ), ReferencerListHead( NULL ) {}
		~ReferenceableClass( void );

		bool	Save( ChunkSaveClass & csave );
		bool	Load( ChunkLoadClass & cload );

	protected:
		ReferencerClass	*ReferencerListHead;	

		T * Get_Data( void ) const	{ return ReferenceData; }

	private:
		T							*ReferenceData;	
};


/*
** An object class that can reference an ReferencableClass
*/
class ReferencerClass : public PostLoadableClass {

	public:
		friend class ReferenceableClass<ScriptableGameObj>;
		ReferencerClass( void ) : ReferenceTarget( NULL ), TargetReferencerListNext( NULL ) {}
		ReferencerClass( const ScriptableGameObj * target ) : ReferenceTarget( NULL ), TargetReferencerListNext( NULL ) { *this = target; }
		virtual ~ReferencerClass( void ) { operator = ((const ScriptableGameObj*)NULL); }

		virtual void	On_Post_Load(void) override;

		bool	Save( ChunkSaveClass & csave );
		bool	Load( ChunkLoadClass & cload );

		const ReferencerClass & operator = ( const ReferencerClass & src ) { operator = (src.Get_Ptr()); return *this; }
		const ReferencerClass & operator = ( const ScriptableGameObj * reference_target );

		void	Set_Ptr (const ScriptableGameObj * reference_target) { *this = reference_target; }
		ScriptableGameObj *	Get_Ptr (void) const { return ReferenceTarget ? ReferenceTarget->Get_Data() : NULL; }
		operator	ScriptableGameObj * (void) const { return Get_Ptr(); }

	protected:
		ReferenceableClass<ScriptableGameObj>	*ReferenceTarget;	
		ReferencerClass								*TargetReferencerListNext;	
};


/*
** Clears all references in objects referencing me
*/
template<class T>
ReferenceableClass<T>::~ReferenceableClass( void )	
{
	while ( ReferencerListHead != NULL ) {	// clear each reference to me
		ReferencerClass *referencer = ReferencerListHead;
		WWASSERT( referencer->ReferenceTarget == this );
		ReferencerListHead = referencer->TargetReferencerListNext;
		referencer->ReferenceTarget = NULL;
		referencer->TargetReferencerListNext = NULL;
	}
}


/*
** Save & Load
*/
enum	{
	CHUNKID_REF_VARIABLES				=	913991844,

	XXXX_MICROCHUNKID_LIST_HEAD		=	1,
	MICROCHUNKID_XXXXXXXXX,
	MICROCHUNKID_PTR,
	MICROCHUNKID_TARGET,
	XXXX_MICROCHUNKID_NEXT,
};


template<class T>
bool	ReferenceableClass<T>::Save( ChunkSaveClass & csave )
{
	csave.Begin_Chunk( CHUNKID_REF_VARIABLES );
		void * ptr = this;
		WRITE_MICRO_CHUNK_PTR( csave, MICROCHUNKID_PTR, ptr );
	csave.End_Chunk();
	return true;
}

template<class T>
bool	ReferenceableClass<T>::Load( ChunkLoadClass & cload )
{
	void * old_ptr;
	cload.Open_Chunk();
	WWASSERT( cload.Cur_Chunk_ID() == CHUNKID_REF_VARIABLES );

	WWASSERT( ReferencerListHead == NULL );

	while (cload.Open_Micro_Chunk()) {
		switch(cload.Cur_Micro_Chunk_ID()) {
			case MICROCHUNKID_PTR:
				cload.Read(&old_ptr,sizeof(int));
				SaveLoadSystemClass::Register_Pointer(old_ptr, this);
				break;

			default:
//				Debug_Say(( "Unrecognized REFLIST Variable chunkID\n" ));
				break;
		}
		cload.Close_Micro_Chunk();
	}
	cload.Close_Chunk();

	return true;
}




/*
** A refcounted version of the ReferencerClass
*/
class RefCountedReferencerClass : public ReferencerClass, public RefCountClass {

	public:
		friend class ReferenceableClass<ScriptableGameObj>;
		RefCountedReferencerClass( void ) :
			ReferencerClass ()
		{
		}

		RefCountedReferencerClass( const ReferencerClass &src ) :
			ReferencerClass ()
		{
			ReferencerClass::operator=( src );
		}

};


#endif

