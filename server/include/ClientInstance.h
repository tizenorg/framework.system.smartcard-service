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


#ifndef CLIENTINSTANCE_H_
#define CLIENTINSTANCE_H_

/* standard library header */
#include <map>
//#include <hash_map>

/* SLP library header */

/* local header */
#include "Message.h"
#include "ServiceInstance.h"

namespace smartcard_service_api
{
	class ClientInstance
	{
	private:
		void *ioChannel;
		int socket;
		int watchID;
		int state;
		int pid;
		ByteArray certHash;

		map<unsigned int, ServiceInstance *> mapServices;

		inline ByteArray getCertificationHash() { return certHash; }

	public:
		ClientInstance(void *ioChannel, int socket, int watchID, int state, int pid)
		{
			this->ioChannel = ioChannel;
			this->socket = socket;
			this->watchID = watchID;
			this->state = state;
			this->pid = pid;
		}
		~ClientInstance() { removeServices(); }

		inline bool operator ==(const int &socket) const { return (this->socket == socket); }

		inline void *getIOChannel() { return ioChannel; }
		inline int getSocket() { return socket; }
		inline int getWatchID() { return watchID; }
		inline int getState() { return state; }

		void setPID(int pid);
		inline int getPID() { return pid; }


		bool createService(unsigned int context);
		ServiceInstance *getService(unsigned int context);
		void removeService(unsigned int context);
		void removeServices();

		bool sendMessageToAllServices(int socket, Message &msg);
	};

} /* namespace smartcard_service_api */
#endif /* CLIENTINSTANCE_H_ */
