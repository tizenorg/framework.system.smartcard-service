/*
* Copyright (c) 2012 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/


/* standard library header */
#include <stdio.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "SessionHelper.h"
#include "ReaderHelper.h"

namespace smartcard_service_api
{
	SessionHelper::SessionHelper(ReaderHelper *reader)
	{
		closed = true;

		if (reader == NULL)
			return;

		this->reader = reader;
	}

	SessionHelper::~SessionHelper()
	{
	}

	ReaderHelper *SessionHelper::getReader()
	{
		return reader;
	}

//	ByteArray SessionHelper::getATR()
//	{
//		return atr;
//	}

	bool SessionHelper::isClosed()
	{
		return closed;
	}

} /* namespace smartcard_service_api */
