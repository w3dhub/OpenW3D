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

#include "openw3d.h"
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
   WWASSERT(default_value != nullptr);

   if (registry_location == nullptr) {
      strcpy(RegistryLocation, "");
      strcpy(KeyName, "");
      strcpy(Value, default_value);
      Initialized = true;
   } else {
      WWASSERT(key_name != nullptr);
      WWASSERT(strlen(registry_location) < sizeof(RegistryLocation));
      WWASSERT(strlen(key_name) < sizeof(KeyName));
      strcpy(RegistryLocation, registry_location);
      strcpy(KeyName, key_name);

      strcpy(Value, default_value);
      Initialized = false;
   }
}

//-----------------------------------------------------------------------------
void cRegistryString::Set(const char *value)
{
   WWASSERT(value != nullptr);
   WWASSERT(strlen(value) < sizeof(Value));

   strcpy(Value, value);

   if (strcmp(RegistryLocation, "")) {
	   RegistryClass registry(RegistryLocation);
	   WWASSERT(registry.Is_Valid());
	   registry.Set_String(KeyName, Value);
   }
   Initialized = true;
}

//-----------------------------------------------------------------------------
const char *cRegistryString::Get()
{
	if (!Initialized) {
		WWASSERT(RegistryLocation[0] != '\0');
		WWASSERT(KeyName[0] != '\0');
		RegistryClass registry(RegistryLocation);
		if (!registry.Exists(KeyName)) {
			registry.Set_String(KeyName, Value);
		} else {
			registry.Get_String(KeyName, Value, sizeof(Value), Value);
		}
		Initialized = true;
	}
	return Value;
}
