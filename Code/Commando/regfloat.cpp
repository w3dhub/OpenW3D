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

#include "openw3d.h"
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
	if (registry_location == nullptr) {
      strcpy(RegistryLocation, "");
      strcpy(KeyName, "");
      Value = default_value;
      Initialized = true;
   } else {
      WWASSERT(key_name != nullptr);
      WWASSERT(strlen(registry_location) < sizeof(RegistryLocation));
      WWASSERT(strlen(key_name) < sizeof(KeyName));
      strcpy(RegistryLocation, registry_location);
      strcpy(KeyName, key_name);

      Value = default_value;
      Initialized = false;
   }
}

//-----------------------------------------------------------------------------
void cRegistryFloat::Set(float value)
{
   Value = value;

   if (strcmp(RegistryLocation, "") != 0) {
	   RegistryClass registry(RegistryLocation);
	   WWASSERT(registry.Is_Valid());
	   registry.Set_Float(KeyName, Value);
   }
   Initialized = true;
}

//-----------------------------------------------------------------------------
float cRegistryFloat::Get()
{
	if (!Initialized) {
		WWASSERT(RegistryLocation[0] != '\0');
		WWASSERT(KeyName[0] != '\0');
		RegistryClass registry(RegistryLocation);
		if (!registry.Exists(KeyName)) {
			registry.Set_Float(KeyName, Value);
		} else {
			Value = registry.Get_Float(KeyName, Value);
		}
		Initialized = true;
	}
	return Value;
}
