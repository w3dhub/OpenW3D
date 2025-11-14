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
// Filename:     regint.cpp
// Author:       Tom Spencer-Smith
// Date:         Dec 1998
// Description:  
//

#include "regint.h" // I WANNA BE FIRST!

#include "string.h"
#include "registry.h"
#include "wwdebug.h"
#include "wwmemlog.h"

//
// Class statics
//

//-----------------------------------------------------------------------------
cRegistryInt::cRegistryInt(const char *registry_location, const char *key_name, int default_value)
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
      Value = registry->Get_Int(KeyName, default_value);
   	delete registry;

      Set(Value);
   }
}

//-----------------------------------------------------------------------------
void cRegistryInt::Set(int value)
{
   Value = value;

   if (strcmp(RegistryLocation, "")) {
	   RegistryClass * registry = new RegistryClass(RegistryLocation);
	   WWASSERT(registry != NULL && registry->Is_Valid());
      registry->Set_Int(KeyName, Value);
   	delete registry;
   }
}
