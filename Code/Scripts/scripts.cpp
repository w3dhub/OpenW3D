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

/******************************************************************************
*
* FILE
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Byon_g $
*     $Revision: 85 $
*     $Modtime: 7/05/01 12:12p $
*     $Archive: /Commando/Code/Scripts/scripts.cpp $
*
******************************************************************************/

#include "scripts.h"
#include "ScriptFactory.h"
#include "DPrint.h"
#include "strtrim.h"
#include <string.h>
#include <stdio.h>

#define FOURCC(A,B,C,D) (((A) << 0) | ((B) << 8) | ((C) << 16) | ((D) << 24))

#define	CHUNKID_SCRIPTHEADER			FOURCC('s','h','d','r')
#define	CHUNKID_SCRIPTDATA				FOURCC('s','d','a','t')
#define	CHUNKID_SCRIPT_AUTO_VARIABLES	FOURCC('c','s','a','v')

enum {
	DATAID_PARAMETER_STRING = 1,
	DATAID_OWNERPTR,
};

// Script commands
ScriptCommands* Commands = NULL;
void (*ScriptImpClass::Request_Destroy_Script)(ScriptClass*) =  NULL;

void ScriptImpClass::Set_Request_Destroy_Func(void (*function)(ScriptClass*))
{
	Request_Destroy_Script = function;
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::ScriptImpClass
*
* DESCRIPTION
*     Constructor for concrete script implementation class
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

ScriptImpClass::ScriptImpClass()
	: mOwner(NULL),
		mArgC(0),
		mArgV(NULL),
		AutoVariableList( NULL )
{
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::~ScriptImpClass
*
* DESCRIPTION
*     Destructor
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

ScriptImpClass::~ScriptImpClass()
{
	#ifdef _DEBUG
	if (Commands != NULL) {
		DebugPrint("Script '%s' for object '%d' deleted.\n",
			Get_Name(), ((mOwner != NULL) ? Commands->Get_ID(mOwner) : 0 ));
	}
	#endif

	Clear_Parameters();

	// Delete the auto variables
	while ( AutoVariableList != NULL ) {
		ScriptVariableClass * next = AutoVariableList->Get_Next();
		delete AutoVariableList;
		AutoVariableList = next;
	}
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::Get_Name
*
* DESCRIPTION
*     Retrieve name of script
*
* INPUTS
*     NONE
*
* RESULTS
*     Name - Script name string.
*
******************************************************************************/

const char* ScriptImpClass::Get_Name(void)
{
	return mFactory->GetName();
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::Destroy
*
* DESCRIPTION
*     Destroy script
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

void ScriptImpClass::Destroy_Script(void)
{
	if (Request_Destroy_Script != NULL) {
		Request_Destroy_Script(this);
	}
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::Attach
*
* DESCRIPTION
*     Handle attachment of script to an object
*
* INPUTS
*     Object - Object script is attaching to.
*
* RESULTS
*     NONE
*
******************************************************************************/

void ScriptImpClass::Attach(GameObject* obj)
{
	#ifdef _DEBUG
	if (Commands != NULL) {
		DebugPrint("Attaching script '%s' to object %d\n",
			Get_Name(), Commands->Get_ID(obj));
	}
	#endif

	mOwner = obj;
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::Detach
*
* DESCRIPTION
*     Handle detachment of script from an object.
*
* INPUTS
*     Object - Object script is detaching from.
*
* RESULTS
*     NONE
*
******************************************************************************/

void ScriptImpClass::Detach(GameObject* /* obj */)
{
	mOwner = NULL;
	Destroy_Script();
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::Set_Parameters_String
*
* DESCRIPTION
*     Set script parameters.
*
* INPUTS
*     Params - Parameter string
*
* RESULTS
*     NONE
*
******************************************************************************/

void ScriptImpClass::Set_Parameters_String(const char* params)
{
	if ((params == NULL) || (strlen(params) == 0)) {
		return;
	}

	DebugPrint("Setting parameters for script '%s', params = '%s'\n",
		Get_Name(), params);

	char* working = strdup(params);

	if ( strlen( working ) && working[ strlen( working ) - 1 ] == '\n' ) {
		working[ strlen( working ) - 1 ] = 0;
	}

	// Count the parameters
	int count = 0;
	char * param_ptr = working;
	if ( *param_ptr ) count++;
	for ( param_ptr = working; *param_ptr; param_ptr++ ) {
		if ( *param_ptr == ',' )	count++;
	}

	if (count == 0) {
		return;
	}

	// Release old argument vector
	Clear_Parameters();

	// Create new argument vector
	mArgC = count;
	mArgV = new char*[count];
	assert(mArgV != NULL);
	memset(mArgV, 0, (sizeof(char*) * count));

	// Copy the parameters
	strcpy(working, params);

	count = 0;
	param_ptr = working;
	while ( *param_ptr ) {
		// Find where this parameter ends
		char * next = param_ptr;
		while ( *next && *next != ',' ) next++;
		if ( *next ) *next++ = 0;

		// Set param
		Set_Parameter(count++, param_ptr);

		param_ptr = next;
	}

	if (count == mArgC - 1) {
		Set_Parameter(count, "");
	}

	free(working);
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::Get_Parameters_String
*
* DESCRIPTION
*     Retrieve a copy of the scripts parameters string.
*
* INPUTS
*     Buffer - Buffer to copy parameter string into.
*     Size   - Size of buffer.
*
* RESULTS
*     NONE
*
******************************************************************************/

void ScriptImpClass::Get_Parameters_String(char* buffer, unsigned int size)
{
	assert(buffer != NULL);

	buffer[0] = '\0';

	// Build parameter string (comma seperated)
	int count = Get_Parameter_Count();
	int i = 0;
	size_t stringLen = 0;
	const size_t buffer_size = size;

	while ((i < count) && (stringLen < buffer_size)) {
		if (i > 0) {
			stringLen++;
			strcat(buffer, ",");
		}

		const char* param = Get_Parameter(i);
		stringLen += ::strlen(param);

		if (stringLen <= buffer_size) {
			strcat(buffer, param);
			i++;
		}
	}
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::Clear_Parameters
*
* DESCRIPTION
*     Release script parameters
*
* INPUTS
*     NONE
*
* RESULTS
*     NONE
*
******************************************************************************/

void ScriptImpClass::Clear_Parameters(void)
{
	if (mArgV != NULL) {
		while (mArgC--) {
			if (mArgV[mArgC] != NULL) {
				free((void*)mArgV[mArgC]);
			}
		}

		delete[] mArgV;
	}

	mArgC = 0;
	mArgV = NULL;
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::Set_Parameter
*
* DESCRIPTION
*     Set a script parameter
*
* INPUTS
*     Index - Ordinal position of parameter
*     Param - Parameter string
*
* RESULTS
*     NONE
*
******************************************************************************/

void ScriptImpClass::Set_Parameter(int index, const char* str)
{
	// Release old parameter
	if (mArgV[index] != NULL) {
		free(mArgV[index]);
	}

	mArgV[index] = strdup(str);
	assert(mArgV[index] != NULL);
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::Get_Parameter
*
* DESCRIPTION
*     Retrieve paramter by name
*
* INPUTS
*     Name - Paramter name
*
* RESULTS
*     Param - Parameter
*
******************************************************************************/

const char* ScriptImpClass::Get_Parameter(const char* name)
{
	int index = Get_Parameter_Index(name);
	return Get_Parameter(index);
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::Get_Parameter
*
* DESCRIPTION
*     Retrieve parameter at specified index.
*
* INPUTS
*     Index - Ordinal position of paramater
*
* RESULTS
*     Param - Parameter string.
*
******************************************************************************/

const char* ScriptImpClass::Get_Parameter(int index)
{
	if ((index < 0) || (index >= mArgC)) {
		return "";
	}

	return mArgV[index];
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::Get_Int_Parameter
*
* DESCRIPTION
*     Retrieve parameter as an integer.
*
* INPUTS
*     Name - Name of parameter
*
* RESULTS
*     Value - Integer value.
*
******************************************************************************/

int ScriptImpClass::Get_Int_Parameter(const char* parameterName)
{
	int index = Get_Parameter_Index(parameterName);
	return Get_Int_Parameter(index);
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::Get_Float_Parameter
*
* DESCRIPTION
*     Retrieve parameter as a floating point number.
*
* INPUTS
*     Name - Name of parameter
*
* RESULTS
*     Value - Floating point value
*
******************************************************************************/

float ScriptImpClass::Get_Float_Parameter(const char* parameterName)
{
	int index = Get_Parameter_Index(parameterName);
	return Get_Float_Parameter(index);
}


/******************************************************************************
**
******************************************************************************/
Vector3 ScriptImpClass::Get_Vector3_Parameter( int index )
{
	float		x,y,z;
	::sscanf( Get_Parameter( index ), "%f %f %f", &x, &y, &z );
	return Vector3( x,y,z );
}


Vector3 ScriptImpClass::Get_Vector3_Parameter(const char* parameterName)
{
	int index = Get_Parameter_Index(parameterName);
	return Get_Vector3_Parameter(index);
}



/******************************************************************************
*
* NAME
*     ScriptImpClass::Get_Parameter_Index
*
* DESCRIPTION
*     Obtain index of parameter with specified name.
*
* INPUTS
*     Name - Name of parameter
*
* RESULTS
*     Index - Ordinal position of parameter
*
******************************************************************************/

int ScriptImpClass::Get_Parameter_Index(const char* parameterName)
{
	const char* paramDesc = mFactory->GetParamDescription();

	// Make copy of parameter description string to work with.
	char string[512];
	strncpy(string, paramDesc, (sizeof(string) - 1));
	string[sizeof(string)-1] = '\0';

	// Search for the specified parameter
	int index = 0;
	char * param_ptr = string;
	while ( *param_ptr ) {
		// Find where this parameter ends
		char * next = param_ptr;
		while ( *next && *next != ',' ) next++;
		if ( *next ) *next++ = 0;

		// Strip the name portion from the parameter definition
		char* tokenEnd = strpbrk(param_ptr, "=:\n");

		if (tokenEnd != NULL) {
			*tokenEnd = '\0';
		}

		// Check for specified parameter
		strtrim(param_ptr);

		if (stricmp(param_ptr, parameterName) == 0) {
			return index;
		}

		// Advance to next parameter definition
		index++;

		param_ptr = next;
	}

	return -1;
}


/******************************************************************************
*
* NAME
*     ScriptImpClass::Save
*
* DESCRIPTION
*     Save script data
*
* INPUTS
*     ScriptSaver& saver
*
* RESULTS
*     NONE
*
******************************************************************************/

void ScriptImpClass::Save(ScriptSaver& saver)
{
/*	Commands->Begin_Chunk(saver, CHUNKID_SCRIPTDATA);
	Save_Data(saver);
	Commands->End_Chunk(saver);
*/

	ScriptVariableClass * var = AutoVariableList;
	if ( var != NULL ) {
		Commands->Begin_Chunk(saver, CHUNKID_SCRIPT_AUTO_VARIABLES);
	 	while ( var != NULL ) {
			Commands->Save_Data(saver, var->Get_ID(), var->Get_Data_Size(), var->Get_Data_Ptr() );
	 		var = var->Get_Next();
	 	}
		Commands->End_Chunk(saver);
	}

}


/******************************************************************************
*
* NAME
*     ScriptImpClass::Load
*
* DESCRIPTION
*     Load script data
*
* INPUTS
*     ScriptLoader& loader
*
* RESULTS
*     NONE
*
******************************************************************************/

void ScriptImpClass::Load(ScriptLoader& loader)
{
	unsigned int chunkID;

	while (Commands->Open_Chunk(loader, &chunkID)) {

		switch (chunkID) {

/*			case CHUNKID_SCRIPTDATA:
				DebugPrint("Encountered data chunk.\n");
				Load_Data(loader);
			break;
*/

			case CHUNKID_SCRIPT_AUTO_VARIABLES:
			{
				int id;
				while (Commands->Load_Begin(loader, &id)) {
					// If we find this ID, load it
					ScriptVariableClass * var = AutoVariableList;
					while ( var != NULL ) {
						if ( var->Get_ID() == id ) {
							Commands->Load_Data(loader, var->Get_Data_Size(), var->Get_Data_Ptr() );
						}
				 		var = var->Get_Next();
					}
					Commands->Load_End(loader);
				}
			}
			break;

			default:
				DebugPrint("***** ScriptImpClass::Load() - Unrecognized chunk %0x!\n", chunkID);
			break;
		}

		Commands->Close_Chunk(loader);
	}
}


/*
**
*/
void	ScriptImpClass::Auto_Save_Variable( void * data_ptr, int data_size, int id )
{
	if ( ( id < 0 ) || ( id > 255 ) ) {
		DebugPrint("***** ScriptImpClass::Auto_Save_Variable  Bad ID %d\n", id );
		return;
	}

	// First be sure we don't already have this ID
	ScriptVariableClass * var = AutoVariableList;
	while ( var != NULL ) {
		if ( var->Get_ID() == id ) {
			DebugPrint("***** ScriptImpClass::Auto_Save_Variable  Dupe ID %d\n", id );
			return;
		}
		var = var->Get_Next();
	}

	// Never save more than 250 bytes
	if ( data_size > 250 ) {
		DebugPrint("***** ScriptImpClass::Too Much Save Data on ID %d\n", id );
		return;
	}


	// Then create and add the variable;
	AutoVariableList = new ScriptVariableClass( data_ptr, data_size, id, AutoVariableList );
}
