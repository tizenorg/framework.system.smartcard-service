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


#ifndef IPCHELPER_H_
#define IPCHELPER_H_

/* standard library header */
#include <glib.h>
#include <pthread.h>

/* SLP library header */

/* local header */
#include "Message.h"
#include "DispatcherHelper.h"

namespace smartcard_service_api
{
	class IPCHelper
	{
	protected:
		static const int IPC_SERVER_PORT = 8989;
		static const int IPC_SERVER_MAX_CLIENT = 10;

		int ipcSocket;
		unsigned int watchId;
		GIOChannel *ioChannel;
		pthread_mutex_t ipcLock;
		DispatcherHelper *dispatcher;

		static gboolean channelCallbackFunc(GIOChannel* channel, GIOCondition condition, gpointer data);

		virtual int handleIOErrorCondition(void *channel, GIOCondition condition) = 0;
		virtual int handleInvalidSocketCondition(void *channel, GIOCondition condition) = 0;
		virtual int handleIncomingCondition(void *channel, GIOCondition condition) = 0;

	public:
		IPCHelper();
		~IPCHelper();

		bool createListenSocket();
		bool createConnectSocket();

		bool sendMessage(Message *msg);
		bool sendMessage(int socket, Message *msg);
		Message *retrieveMessage();
		Message *retrieveMessage(int socket);

		void setDispatcher(DispatcherHelper *dispatcher);

		friend gboolean channelCallbackFunc(GIOChannel* channel, GIOCondition condition, gpointer data);
	};

} /* namespace smartcard_service_api */
#endif /* IPCHELPER_H_ */
