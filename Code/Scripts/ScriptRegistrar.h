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

#ifndef _SCRIPTREGISTRAR_H_
#define _SCRIPTREGISTRAR_H_

class ScriptFactory;
class ScriptImpClass;

class ScriptRegistrar
	{
	public:
		// Add ScriptFactory with the registrar
		static void RegisterScript(ScriptFactory* factory);

		// Remove ScriptFactory from the registrar
		static void UnregisterScript(ScriptFactory* factory);

		// Create an instance of a script
		static ScriptImpClass* CreateScript(const char* name);

		// Get ScriptFactory with the specified name
		static ScriptFactory* GetScriptFactory(const char* name);

		// Get the ScriptFactory at the specified index
		static ScriptFactory* GetScriptFactory(int index);

		// Retrieve the number of registered ScriptFactories
		static int Count(void);

	private:
		static ScriptFactory* mScriptFactories;
	};

#endif // _SCRIPTREGISTRAR_H_
