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

#pragma once

#ifndef _MIXPATCHMAKER_H
#define _MIXPATCHMAKER_H

class MixFileFactoryClass;
class MixFileCreator;
class MixCombiningDialogClass;



class MixPatchMakerClass
{
	public:
		MixPatchMakerClass(void);
		void Make_Patch(char *old_input_file, char *new_input_file, char *output_file, char *old_art_dir, char *new_art_dir);
		static unsigned int Do_Stuff(void *param);
		void Thread_Make(void);


	private:
		void Copy_File(MixFileFactoryClass *src_mix, MixFileCreator *dest_mix, char *filename);
		bool Compare_File(MixFileFactoryClass *src_mix, MixFileFactoryClass *dest_mix, char *filename);
		bool Compare_Source_Art_File(char *filename);
		bool Find_File(char *file_name, char *path, char *found_path);

		MixCombiningDialogClass *Dialog;
		char OldInputFile[1024];
		char NewInputFile[1024];
		char OutputFile[1024];
		char OldArtDir[1024];
		char NewArtDir[1024];


};


#endif //_MIXPATCHMAKER_H
