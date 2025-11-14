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
// Filename:     regstring.cpp
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  
//

#include "regstring.h" // I WANNA BE FIRST!

#include "string.h"
#include "registry.h"
#include "wwdebug.h"

//
// Class statics
//

//-----------------------------------------------------------------------------
cRegistryString::cRegistryString(const char *registry_location, const char *key_name, 
	const char *default_value)
{
   WWASSERT(default_value != NULL);

   if (registry_location == NULL) {
      strcpy(RegistryLocation, "");
      strcpy(KeyName, "");
      Set(default_value);
   } else {
      WWASSERT(strlen(registry_location) < sizeof(RegistryLocation));
      WWASSERT(key_name != NULL);
      WWASSERT(strlen(key_name) < sizeof(KeyName));
      strcpy(RegistryLocation, registry_location);
      strcpy(KeyName, key_name);

	   RegistryClass * registry = new RegistryClass(RegistryLocation);
	   WWASSERT(registry != NULL && registry->Is_Valid());
		registry->Get_String(KeyName, Value, sizeof(Value), default_value);
   	delete registry;

      Set(Value);
   }
}

//-----------------------------------------------------------------------------
void cRegistryString::Set(const char *value)
{
   WWASSERT(value != NULL);
   WWASSERT(strlen(value) < sizeof(Value));

   strcpy(Value, value);

   if (strcmp(RegistryLocation, "")) {
	   RegistryClass * registry = new RegistryClass(RegistryLocation);
	   WWASSERT(registry != NULL && registry->Is_Valid());
		registry->Set_String(KeyName, Value);
   	delete registry;
   }
}
