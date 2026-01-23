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
 *                     $Archive:: /Commando/Code/Scripts/scripts.h                      $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/29/01 11:08a                                             $* 
 *                                                                                             * 
 *                    $Revision:: 26                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	SCRIPTS_H
#define	SCRIPTS_H

#include "scriptevents.h"
#include "scriptcommands.h"
#include "ScriptRegistrant.h"
#include "string_ids.h"
#include <cstdint>

class CombatSound;
class ScriptFactory;


// ScriptVariables
class ScriptVariableClass 
{
public:
	ScriptVariableClass( void * data_ptr, int data_size, int id, ScriptVariableClass * next ) :
		DataPtr( data_ptr ), 
		DataSize( data_size ),
		ID( id ),
		Next( next )	{}

	void * 						Get_Data_Ptr( void )		{ return DataPtr; }
	int	 						Get_Data_Size( void )	{ return DataSize; }
	int							Get_ID( void )				{ return ID; }
	ScriptVariableClass *	Get_Next( void )			{ return Next; }

private:
	void *						DataPtr;
	int							DataSize;
	int							ID;
	ScriptVariableClass *	Next;
};


class ScriptImpClass : public ScriptClass
{
public:
	static void Set_Request_Destroy_Func(void (*function)(ScriptClass*));

	ScriptImpClass();
	virtual ~ScriptImpClass();

	// Retrieve the name of the script.
	const char* Get_Name(void) override;

	// Retrieve owner object of the script.
	GameObject* Owner() override
		{return mOwner;}

	GameObject** Get_Owner_Ptr() override
		{return &mOwner;}

	// Set the scripts parameter string.
	void Set_Parameters_String(const char* params) override;

	// Retrieve the scripts parameter string.
	void Get_Parameters_String(char* buffer, unsigned int size) override;

	// Retrieve the parameter count.
	int Get_Parameter_Count(void)
		{return mArgC;}

	// Retrieve a parameter by name.
	const char* Get_Parameter(const char* name);

	// Retrieve a parameter by is ordinal position in the parameter list.
	const char* Get_Parameter(int index);

	// Get a parameter as an integer
	int Get_Int_Parameter(int index)
		{return atoi(Get_Parameter(index));}

	// Get a parameter as an integer
	int Get_Int_Parameter(const char* parameterName);

	// Get a parameter as a float
	float Get_Float_Parameter(int index)
		{return (float)atof(Get_Parameter(index));}

	// Get a parameter as a float
	float Get_Float_Parameter(const char* parameterName);

	// Get a parameter as a vector3
	Vector3 Get_Vector3_Parameter(int index);

	// Get a parameter as a float
	Vector3 Get_Vector3_Parameter(const char* parameterName);

	// Get a parameter as an float
	int Get_Parameter_Index(const char* parameterName);

	// Set the script factory used to create this script.
	//
	// This must only be called by the script factory upon creation of
	// the script.
	void SetFactory(ScriptFactory* factory)
		{mFactory = factory;}

	/* Event Functions which will be called as events happen
	 * Scripts can choose to override any of these functions,
	 * otherwise the following empty functions will be called
	 */
	virtual	void	Created( GameObject * /*obj*/ ) override {}
	virtual	void	Destroyed( GameObject * /*obj*/ ) override {}
	virtual	void	Killed( GameObject * /*obj*/, GameObject * /*killer*/ ) override {}
	virtual	void	Damaged( GameObject * /*obj*/, GameObject * /*damager*/, float /*amount*/ ) override {}
	virtual	void	Custom( GameObject * /*obj*/, int /*type*/, intptr_t /*param*/, GameObject * /*sender*/ ) override {}
	virtual	void	Sound_Heard( GameObject * /*obj*/, const CombatSound & /*sound*/ ) override {}
	virtual	void	Enemy_Seen( GameObject * /*obj*/, GameObject * /*enemy*/ ) override {}
	virtual	void	Action_Complete( GameObject * /*obj*/, int /*action_id*/, ActionCompleteReason /*complete_reason*/ ) override	{}
	virtual	void	Timer_Expired( GameObject * /*obj*/, int /*timer_id*/ ) override {}
	virtual	void	Animation_Complete( GameObject * /*obj*/, const char * /*animation_name*/ ) override {}
	virtual	void	Poked( GameObject * /*obj*/, GameObject * /*poker*/ ) override {}
	virtual	void	Entered( GameObject * /*obj*/, GameObject * /*enterer*/ ) override {}
	virtual	void	Exited( GameObject * /*obj*/, GameObject * /*exiter*/ ) override {}

	// Save and Load specific script
	virtual void Save_Data(ScriptSaver& /*saver*/) {}
	virtual void Load_Data(ScriptLoader& /*loader*/) {}

	// Auto Variable Save and Load
	virtual	void Register_Auto_Save_Variables( void )	{}
	void	Auto_Save_Variable( void * data_ptr, int data_size, int id );

protected:
	void Destroy_Script(void);

	void Attach(GameObject* obj) override;
	void Detach(GameObject* obj) override;

	virtual void Save(ScriptSaver& saver) override;
	virtual void Load(ScriptLoader& loader) override;

private:
	static void (*Request_Destroy_Script)(ScriptClass*);

	void Clear_Parameters(void);
	void Set_Parameter(int index, const char* str);

	GameObject* mOwner;
	int mArgC;
	char** mArgV;

	// The factory reference is provided to the script class so that it
	// knows what it is (IE: Name, Parameter description).
	//
	// The factory reference could also be used for script cloning.
	// (The script would use this factory to create a new instance of
	// itself then copy its state to the new script instance.)
	ScriptFactory* mFactory;

	// Auto Variable Save and Load
	ScriptVariableClass *	AutoVariableList;
};


// Declare script definition
#define	DECLARE_SCRIPT(x, d) \
	REGISTER_SCRIPT(x, d) \
	class x : public ScriptImpClass

// Load / Save Macros
#define SAVE_BEGIN()
#define SAVE_DATA(id, var) Commands->Save_Data(saver, id, sizeof(var), &var)
#define SAVE_STRING(id, string) Commands->Save_Data(saver, id, strlen(string), string)
#define SAVE_END()


#define LOAD_BEGIN() \
{ \
	int id; \
	while (Commands->Load_Begin(loader, &id)) { \
		switch (id) {

#define LOAD_DATA(id, var) \
			case id: \
				Commands->Load_Data(loader, sizeof(var), &var); \
			break;

#define LOAD_END() \
			default: \
				break; \
		} \
		Commands->Load_End(loader); \
	} \
}

#define LOAD_STRING(id, var) LOAD_DATA(id, var)

// Auto Variable Save/Load
#define REGISTER_VARIABLES()			public: void Register_Auto_Save_Variables( void ) override
#define SAVE_VARIABLE( x, id )		Auto_Save_Variable( &x, sizeof( x ), id )

extern ScriptCommands* Commands;

// Array Macros
#define		ARRAY_ELEMENT_COUNT( x )	( sizeof( x ) / sizeof( x[0] ) )
#define		RANDOM_ARRAY_ELEMENT( x )	( x[Commands->Get_Random_Int( 0, ARRAY_ELEMENT_COUNT( x ) )] )    

#endif // SCRIPTS_H
