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

//
// Filename:     regfloat.cpp
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  
//

#include "regfloat.h"

#include "string.h"
#include "registry.h"
#include "wwdebug.h"
#include "wwmemlog.h"

//
// Class statics
//

//-----------------------------------------------------------------------------
cRegistryFloat::cRegistryFloat(const char *registry_location, const char *key_name, float default_value)
{
   WWMEMLOG(MEM_GAMEDATA);
	if (registry_location == NULL) {
      strcpy(RegistryLocation, "");
      strcpy(KeyName, "");
      Set(default_value);
   } else {
      WWASSERT(key_name != NULL);
      WWASSERT(strlen(registry_location) < sizeof(RegistryLocation));
      WWASSERT(strlen(key_name) < sizeof(KeyName));
      strcpy(RegistryLocation, registry_location);
      strcpy(KeyName, key_name);

	   RegistryClass * registry = new RegistryClass(RegistryLocation);
	   WWASSERT(registry != NULL && registry->Is_Valid());
		int temp_1 = 0;
		WWASSERT(sizeof(temp_1) == sizeof(default_value));
		::memcpy(&temp_1, &default_value, sizeof(default_value));
		int temp_2 = registry->Get_Int(KeyName, temp_1);
		WWASSERT(sizeof(temp_2) == sizeof(Value));
		::memcpy(&Value, &temp_2, sizeof(temp_2));
   	delete registry;

      Set(Value);
   }
}

//-----------------------------------------------------------------------------
void cRegistryFloat::Set(float value)
{
   Value = value;

   if (strcmp(RegistryLocation, "")) {
	   RegistryClass * registry = new RegistryClass(RegistryLocation);
	   WWASSERT(registry != NULL && registry->Is_Valid());
		int temp = 0;
		WWASSERT(sizeof(temp) == sizeof(Value));
		::memcpy(&temp, &Value, sizeof(Value));
      registry->Set_Int(KeyName, temp);
   	delete registry;
   }
}
