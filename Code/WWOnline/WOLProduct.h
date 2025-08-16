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
*     $Archive: /Commando/Code/WWOnline/WOLProduct.h $
*
* DESCRIPTION
*     This class specifies product-specific information, such as SKU.
*
*     Client code should create a Product::Initializer object. This will create
*     a Product object and set it as the current product. Creating additional
*     Initializer objects will replace the current product information. This
*     will change the application's "identity" on the fly; this may cause
*     problems if Westwood Online activity is in progress. I don't expect there
*     to be any need to do this, except perhaps during early product development.
*
* PROGRAMMER
*     $Author: Denzil_l $
*
* VERSION INFO
*     $Revision: 4 $
*     $Modtime: 1/25/02 6:45p $
*
******************************************************************************/

#ifndef __WOLPRODUCT_H__
#define __WOLPRODUCT_H__

#include "RefCounted.h"
#include "RefPtr.h"
#include <wwlib/WWString.h>
#include <wwlib/WideString.h>

namespace WWOnline {

class Product :
		public RefCounted
	{
	public:
		static RefPtr<Product> Current(void);

		const char* GetRegistryPath(void) const
			{return mRegistryPath;}

		unsigned long GetSKU(void) const
			{return mProductSKU;}

		unsigned long GetLanguageSKU(void) const
			{return mProductSKU | mLanguageCode;}

		unsigned long GetLadderSKU(void) const
			{return mLadderSKU;}

		unsigned long GetLanguageCode(void) const
			{return mLanguageCode;}

		unsigned long GetVersion(void) const
			{return mProductVersion;}

		long GetGameCode(void) const
			{return mGameCode;}

		const wchar_t* GetChannelPassword(void) const
			{return mChannelPassword;}

		class Initializer
			{
			public:
				Initializer(const char* registryPath, int gameCode, const wchar_t* chanPass, unsigned long ladderSKU);
				~Initializer();
			};

	private:
		friend class Initializer;
		static RefPtr<Product> Create(const char* registryPath, int gameCode, const wchar_t* chanPass, unsigned long ladderSKU);

		Product(const char* registryPath, int gameCode, const wchar_t* chanPass, unsigned long ladderSKU);

		StringClass mRegistryPath;
		unsigned long mProductSKU;
		unsigned long mProductVersion;
		unsigned long mLanguageCode;
		unsigned long mLadderSKU;
		long mGameCode;
		WideStringClass mChannelPassword;
	};

}
#endif // __WOLPRODUCT_H__
