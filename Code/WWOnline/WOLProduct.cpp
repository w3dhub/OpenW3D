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
*     $Archive: /Commando/Code/WWOnline/WOLProduct.cpp $
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

#include "WOLProduct.h"
#include <WWLib\win.h>

namespace WWOnline {

/******************************************************************************
*
* NAME
*     CurrentProduct
*
* DESCRIPTION
*     Current WWOnline product.
*
* INPUTS
*     NONE
*
* RESULT
*     Product -
*
******************************************************************************/

static RefPtr<Product>& CurrentProduct(void)
	{
	static RefPtr<Product> _current;
	return _current;
	}


/******************************************************************************
*
* NAME
*     Product::Current
*
* DESCRIPTION
*     Get the current WWOnline product
*
* INPUTS
*     NONE
*
* RESULT
*
******************************************************************************/

RefPtr<Product> Product::Current(void)
	{
	return CurrentProduct().ReferencedObject();
	}


/******************************************************************************
*
* NAME
*     Product::Create
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

RefPtr<Product> Product::Create(const char* registryPath, int gameCode,
		const wchar_t* chanPass, unsigned long ladderSKU)
	{
	return new Product(registryPath, gameCode, chanPass, ladderSKU);
	}


/******************************************************************************
*
* NAME
*     Product::Product
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

Product::Product(const char* registryPath, int gameCode, const wchar_t* chanPass, unsigned long ladderSKU) :
		mRegistryPath(registryPath),
		mProductSKU(0),
		mLadderSKU(0),
		mProductVersion(0),
		mLanguageCode(0),
		mGameCode(gameCode),
		mChannelPassword(chanPass)
	{
	WWASSERT(registryPath && "Invalid parameter");

	HKEY rKey;
	LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, registryPath, 0, KEY_READ, &rKey);
	
	if (result == ERROR_SUCCESS)
		{
		// Get SKU
		DWORD type;
		DWORD sku = 0;
		DWORD sizeOfBuffer = sizeof(sku);
		result = RegQueryValueEx(rKey, "SKU", NULL, &type, (unsigned char*)&sku, &sizeOfBuffer);

		mProductSKU = sku;
		mLanguageCode = (sku & 0xFF);
		mLadderSKU = ladderSKU;

		// Get version
		DWORD version = 0;
		sizeOfBuffer = sizeof(version);
		result = RegQueryValueEx(rKey, "Version", NULL, &type, (unsigned char*)&version, &sizeOfBuffer);

		mProductVersion = version;

		RegCloseKey(rKey);
		}
	}


/******************************************************************************
*
* NAME
*     WOLProduct::Initializer
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

Product::Initializer::Initializer(const char* registryPath, int gameCode,
		const wchar_t* chanPass, unsigned long ladderSKU)
	{
	CurrentProduct() = Product::Create(registryPath, gameCode, chanPass, ladderSKU);
	}


/******************************************************************************
*
* NAME
*     WOLProduct::Initializer
*
* DESCRIPTION
*
* INPUTS
*
* RESULT
*
******************************************************************************/

Product::Initializer::~Initializer()
	{
	}

}
