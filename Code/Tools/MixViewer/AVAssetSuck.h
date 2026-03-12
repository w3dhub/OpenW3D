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

#ifndef _AVASSETSUCK_H
#define _AVASSETSUCK_H

class MixFileFactoryClass;
class MixFileCreator;
class MixCombiningDialogClass;



class AVAssetSuckerClass
{
	public:
		AVAssetSuckerClass(void);
		void Suck(char *input_file, char *output_file);
		static unsigned int Do_Stuff(void *param);
		void Thread_Suck(void);


	private:
		void Copy_File(MixFileFactoryClass *src_mix, MixFileCreator *dest_mix, char *filename);

		MixCombiningDialogClass *Dialog;
		char InputFile[1024];
		char OutputFile[1024];



};


#endif //_AVASSETSUCK_H
