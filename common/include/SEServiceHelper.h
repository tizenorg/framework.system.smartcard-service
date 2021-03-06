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


#ifndef SESERVICEHELPER_H_
#define SESERVICEHELPER_H_

/* standard library header */
#include <vector>

/* SLP library header */

/* local header */
#include "Synchronous.h"
#include "ReaderHelper.h"

using namespace std;

namespace smartcard_service_api
{
	class SEServiceHelper : public Synchronous
	{
	protected:
		bool connected;
		vector<ReaderHelper *> readers;

	public:
		SEServiceHelper();
		~SEServiceHelper();

		vector<ReaderHelper *> getReaders();
		bool isConnected();
		void shutdown();
	};

} /* namespace smartcard_service_api */
#endif /* SESERVICEHELPER_H_ */
