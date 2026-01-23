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
 *                 Project Name : combat																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/statemachine.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 11/07/01 5:27p                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __STATEMACHINE_H
#define __STATEMACHINE_H


#include "simplevec.h"
#include "chunkio.h"
#include "wwdebug.h"


//////////////////////////////////////////////////////////////////////
//	Macros
//////////////////////////////////////////////////////////////////////
//#define ADD_STATE_TO_MACHINE(machine, state)		\
		//machine.Add_State (On_##state_Think, On_##state_Request_End, On_##state_Begin, On_##state_End);

/*#define ADD_STATE_TO_MACHINE(machine, state, is_think, is_request_end, is_begin, is_end)		\
		machine.Add_State (															\
			is_think ? On_##state##_Think : NULL,									\
			is_request_end ? On_##state##_Request_End : NULL,					\
			is_begin ? On_##state##_Begin : NULL,									\
			is_end ? On_##state##_End : NULL);	*/

#define ADD_STATE_TO_MACHINE(base, machine, state)		\
		machine.Add_State (								\
			&base::On_##state##_Think,							\
			&base::On_##state##_Request_End,					\
			&base::On_##state##_Begin,							\
			&base::On_##state##_End);


		//machine.Add_State (On_##state_Think, On_##state_Request_End, On_##state_Begin, On_##state_End);

/*#define BEGIN_STATE_MAP(machine)						\
		void	Install_##machine_States (void) {	\
			

#define ADD_STATE_HANDLER(machine, state)		\
			machine.Add_State (state);				\
			machine.
			

#define END_STATE_MAP()					\
		}*/

//#define ADD_STATE_TO_MACHINE(machine, state)		\
		//machine.Add_State (On_##state_Think, On_##state_Request_End, On_##state_Begin, On_##state_End);


/*#define DECLARE_STATE_HANDLER(state)						\
		void On_##state##_Begin (void);						\
		void On_##state##_End (void);							\
		void On_##state##_Think (void);						\
		bool On_##state##_Request_End (int new_state);*/

#define SM_BEGIN			;
#define SM_END				;
#define SM_THINK			;
#define SM_REQ_END		;
#define SM_NO_BEGIN		{}
#define SM_NO_END			{}
#define SM_NO_THINK		{}
#define SM_NO_REQ_END	{ return true; }

#define DECLARE_STATE(state, begin, end, think, req_end)			\
		void On_##state##_Begin (void) begin					\
		void On_##state##_End (void) end							\
		void On_##state##_Think (void) think					\
		bool On_##state##_Request_End ([[maybe_unused]] int state) req_end

#define DECLARE_STATE_HANDLER_BEGIN(state)			\
		void On_##state##_Begin (void)

#define DECLARE_STATE_HANDLER_END(state)				\
		void On_##state##_End (void)

#define DECLARE_STATE_HANDLER_THINK(state)			\
		void On_##state##_Think (void)

#define DECLARE_STATE_HANDLER_REQUEST_END(state)	\
		bool On_##state##_Request_End (int state)

#define STATE_IMPL_BEGIN(state)	\
		On_##state##_Begin

#define STATE_IMPL_END(state)	\
		On_##state##_End

#define STATE_IMPL_THINK(state)	\
		On_##state##_Think

#define STATE_IMPL_REQUEST_END(state)	\
		On_##state##_Request_End



/*#define DECLARE_STATE_HANDLER_BEGIN(state)		\
		void On_##state_Begin (void);

#define DECLARE_STATE_HANDLER_END(state)			\
		void On_##state_End (void);

#define DECLARE_STATE_HANDLER_THINK(state)		\
		void On_##state_Think (void);

#define DECLARE_STATE_HANDLER_REQUEST_END(state)		\
		void On_##state_Request_End (void);

*/

//////////////////////////////////////////////////////////////////////
//
//	StateClass
//
//////////////////////////////////////////////////////////////////////
template <class T>
class StateClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	StateClass (void)	:
		Think (NULL),
		RequestEnd (NULL),
		Begin (NULL),
		End (NULL)				{}

	///////////////////////////////////////////////////////////////////
	//	Public typedefs
	///////////////////////////////////////////////////////////////////
	typedef void (T::*THINK_PTR)			(void);
	typedef bool (T::*REQUEST_END_PTR)	(int new_state);
	typedef void (T::*BEGIN_PTR)			(void);
	typedef void (T::*END_PTR)				(void);

	///////////////////////////////////////////////////////////////////
	//	Public member data
	///////////////////////////////////////////////////////////////////
	THINK_PTR			Think;
	REQUEST_END_PTR	RequestEnd;
	BEGIN_PTR			Begin;
	END_PTR				End;
};


//////////////////////////////////////////////////////////////////////
//
//	StateMachineClass
//
//////////////////////////////////////////////////////////////////////
template <class T>
class StateMachineClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public typedefs
	///////////////////////////////////////////////////////////////////
	typedef StateClass<T>	STATE_OBJ;

	///////////////////////////////////////////////////////////////////
	//	Save/load constants
	///////////////////////////////////////////////////////////////////
	enum
	{
		CHUNKID_VARIABLES		= 0x11070946,

		VARID_CURR_STATE		= 0,
		VARID_IS_HALTED,
	};

	///////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////////////
	StateMachineClass (T *object)	:
		CurrState (-1),
		Object (object),
		IsHalted (false)						{}
	
	virtual ~StateMachineClass (void)	{}

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////
	//	Think
	///////////////////////////////////////////////////////////////////
	void	Think (void)
	{
		if (IsHalted) {
			return ;
		}

		//
		//	Call the think member function of the container
		//
		if (Is_Valid_State (CurrState)) {
			if (StateTable[CurrState].Think != NULL) {
				(Object->*(StateTable[CurrState].Think)) ();
			}
		}

		return ;
	}

	///////////////////////////////////////////////////////////////////
	//	Add_State
	///////////////////////////////////////////////////////////////////
	void	Add_State (StateClass<T> &state)
	{
		StateTable.Add (state);
		return ;
	}

	///////////////////////////////////////////////////////////////////
	//	Add_State
	///////////////////////////////////////////////////////////////////
	void	Add_State
	(
		typename STATE_OBJ::THINK_PTR think_ptr,
		typename STATE_OBJ::REQUEST_END_PTR request_ptr,
		typename STATE_OBJ::BEGIN_PTR begin_ptr,
		typename STATE_OBJ::END_PTR end_ptr
	)
	{
		StateClass<T> state;
		state.Think			= think_ptr;
		state.RequestEnd	= request_ptr;
		state.Begin			= begin_ptr;
		state.End			= end_ptr;

		Add_State (state);
		return ;
	}

	///////////////////////////////////////////////////////////////////
	//	Halt_State
	///////////////////////////////////////////////////////////////////
	void	Halt_State (void)
	{
		if (IsHalted) {
			return ;
		}

		//
		//	Notify the current state that it's "ending"
		//
		if (Is_Valid_State (CurrState)) {
			
			if (StateTable[CurrState].End != NULL) {
				(Object->*(StateTable[CurrState].End)) ();
			}
		}
		
		IsHalted = true;
		return ;
	}

	///////////////////////////////////////////////////////////////////
	//	Resume_State
	///////////////////////////////////////////////////////////////////
	void	Resume_State (void)
	{
		if (IsHalted == false) {
			return ;
		}

		//
		//	Notify the current state that it's starting again
		//
		if (Is_Valid_State (CurrState)) {			
			if (StateTable[CurrState].Begin != NULL) {
				(Object->*(StateTable[CurrState].Begin)) ();
			}
		}
		
		IsHalted = false;
		return ;
	}

	///////////////////////////////////////////////////////////////////
	//	Get_State
	///////////////////////////////////////////////////////////////////
	int	Get_State (void)
	{
		return CurrState;
	}

	///////////////////////////////////////////////////////////////////
	//	Set_State
	///////////////////////////////////////////////////////////////////
	void	Set_State (int state_index, bool force = false)
	{
		if (state_index == CurrState && force == false) {
			return ;
		}

		//
		//	First, check to see if we can switch states
		//
		if (IsHalted == false && Is_Valid_State (CurrState)) {
			if (force == false && StateTable[CurrState].RequestEnd != NULL) {
				if ((Object->*(StateTable[CurrState].RequestEnd)) (state_index) == false) {
					return ;
				}
			}

			//
			//	Now, notify the old state that it's ending
			//
			if (StateTable[CurrState].End != NULL) {
				(Object->*(StateTable[CurrState].End)) ();
			}
		}

		if (Is_Valid_State (state_index)) {
						
			//
			//	Switch to the new state
			//
			CurrState = state_index;
			if (IsHalted == false && StateTable[CurrState].Begin != NULL) {
				(Object->*(StateTable[CurrState].Begin)) ();
			}
		} else {
			CurrState = -1;
		}

		return ;
	}

	///////////////////////////////////////////////////////////////////
	//	Save
	///////////////////////////////////////////////////////////////////
	void Save (ChunkSaveClass &csave)
	{
		csave.Begin_Chunk (CHUNKID_VARIABLES);
			WRITE_MICRO_CHUNK (csave, VARID_CURR_STATE,	CurrState);
			WRITE_MICRO_CHUNK (csave, VARID_IS_HALTED,	IsHalted);
		csave.End_Chunk ();		
		return ;
	}

	///////////////////////////////////////////////////////////////////
	//	Load
	///////////////////////////////////////////////////////////////////
	void Load (ChunkLoadClass &cload)
	{
		while (cload.Open_Chunk ())
		{
			switch (cload.Cur_Chunk_ID ())
			{
				case CHUNKID_VARIABLES:
					Load_Variables (cload);
					break;
		  
				default:
					WWDEBUG_SAY (("Unrecognized StateMachineClass chunkID\n"));
					break;

			}
			cload.Close_Chunk ();
		}

		return ;
	}

	///////////////////////////////////////////////////////////////////
	//	Load_Variables
	///////////////////////////////////////////////////////////////////
	void Load_Variables (ChunkLoadClass &cload)
	{
		while (cload.Open_Micro_Chunk ()) {
			switch (cload.Cur_Micro_Chunk_ID ())
			{
				READ_MICRO_CHUNK_UNCHECKED (cload, VARID_CURR_STATE,	this->CurrState);
				READ_MICRO_CHUNK_UNCHECKED (cload, VARID_IS_HALTED,		this->IsHalted);
				
				default:
					WWDEBUG_SAY (("Unrecognized StateMachineClass variable chunkID\n"));
					break;
			}
			cload.Close_Micro_Chunk ();
		}

		return ;
	}

protected:
	
	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////
	//	Is_Valid_State
	///////////////////////////////////////////////////////////////////
	bool	Is_Valid_State (int index)
	{
		return (index >= 0 && index < StateTable.Count ());
	}

	///////////////////////////////////////////////////////////////////
	//	Protected member data
	///////////////////////////////////////////////////////////////////
	SimpleDynVecClass<STATE_OBJ>	StateTable;
	T *									Object;
	int									CurrState;
	bool									IsHalted;
};


#endif //__STATEMACHINE_H
