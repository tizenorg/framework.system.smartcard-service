/*
* Copyright (c) 2012, 2013 Samsung Electronics Co., Ltd.
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
#include <sys/socket.h>
#include <unistd.h>

/* SLP library header */
#ifdef SECURITY_SERVER
#include "security-server.h"
#endif

/* local header */
#include "Debug.h"
#include "ClientIPC.h"
#include "DispatcherMsg.h"

namespace smartcard_service_api
{
	ClientIPC::ClientIPC():IPCHelper()
	{
#ifdef USE_AUTOSTART
		_launch_daemon();
#endif
#ifdef SECURITY_SERVER
		int length = 0;

		if ((length = security_server_get_cookie_size()) > 0)
		{
			uint8_t *buffer = NULL;

			buffer = new uint8_t[length];
			if (buffer != NULL)
			{
				int error = 0;

				if ((error = security_server_request_cookie(buffer, length)) == SECURITY_SERVER_API_SUCCESS)
				{
					cookie.setBuffer(buffer, length);

					SCARD_DEBUG("cookie : %s", cookie.toString());
				}
				else
				{
					SCARD_DEBUG_ERR("security_server_request_cookie failed [%d]", error);
				}

				delete []buffer;
			}
		}
		else
		{
			SCARD_DEBUG_ERR("security_server_get_cookie_size failed");
		}
#endif
	}

	ClientIPC::~ClientIPC()
	{
	}

	ClientIPC &ClientIPC::getInstance()
	{
		static ClientIPC clientIPC;

		return clientIPC;
	}

#ifdef USE_AUTOSTART
	void ClientIPC::_launch_daemon()
	{
		DBusGConnection *connection = NULL;
		DBusGProxy *proxy = NULL;
		GError *error = NULL;
		gint result = 0;

		SCARD_BEGIN();

		dbus_g_thread_init();

		g_type_init();

		connection = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
		if (error == NULL)
		{
			proxy = dbus_g_proxy_new_for_name(connection, "org.tizen.smartcard_service", "/org/tizen/smartcard_service", "org.tizen.smartcard_service");
			if (proxy != NULL)
			{
				if (dbus_g_proxy_call(proxy, "launch", &error, G_TYPE_INVALID, G_TYPE_INT, &result, G_TYPE_INVALID) == false)
				{
					SCARD_DEBUG_ERR("org_tizen_smartcard_service_launch failed");
					if (error != NULL)
					{
						SCARD_DEBUG_ERR("message : [%s]", error->message);
						g_error_free(error);
					}
				}

				g_object_unref(proxy);
			}
			else
			{
				SCARD_DEBUG_ERR("ERROR: Can't make dbus proxy");
			}
		}
		else
		{
			SCARD_DEBUG_ERR("ERROR: Can't get on system bus [%s]", error->message);
			g_error_free(error);
		}

		SCARD_END();
	}
#endif

	bool ClientIPC::sendMessage(Message *msg)
	{
		ByteArray stream;
		unsigned int length = 0;

		if (ipcSocket == -1)
			return false;

#ifdef SECURITY_SERVER
		stream = cookie + msg->serialize();
#else
		stream = msg->serialize();
#endif
		length = stream.getLength();

		SCARD_DEBUG(">>>[SEND]>>> socket [%d], msg [%d], length [%d]", ipcSocket, msg->message, stream.getLength());

		return IPCHelper::sendMessage(ipcSocket, stream);
	}

	int ClientIPC::handleIOErrorCondition(void *channel, GIOCondition condition)
	{
		DispatcherMsg dispMsg;

		SCARD_BEGIN();

		/* push or process disconnect message */
		dispMsg.message = Message::MSG_OPERATION_RELEASE_CLIENT;
		dispMsg.error = -1;

		if (dispatcher != NULL)
		{
#ifdef CLIENT_IPC_THREAD
			dispatcher->processMessage(&dispMsg);
#else
			dispatcher->pushMessage(&dispMsg);
#endif
		}

		SCARD_END();

		return FALSE;
	}

	int ClientIPC::handleInvalidSocketCondition(void *channel, GIOCondition condition)
	{
		SCARD_BEGIN();

		/* finalize context */
		destroyConnectSocket();

		SCARD_END();

		return FALSE;
	}

	int ClientIPC::handleIncomingCondition(void *channel, GIOCondition condition)
	{
		int result = FALSE;

		SCARD_BEGIN();

#ifndef CLIENT_IPC_THREAD
		if (channel == ioChannel)
		{
#endif
			Message *msg = NULL;

			SCARD_DEBUG("message from server to client socket");

			/* read message */
			msg = retrieveMessage();
			if (msg != NULL)
			{
				DispatcherMsg dispMsg(msg);

				/* set peer socket */
				dispMsg.setPeerSocket(ipcSocket);

				/* push to dispatcher */
				if (dispatcher != NULL)
				{
#ifdef CLIENT_IPC_THREAD
					dispatcher->processMessage(&dispMsg);
#else
					dispatcher->pushMessage(&dispMsg);
#endif
				}

				delete msg;
			}
			else
			{
				/* clear client connection */
#ifdef CLIENT_IPC_THREAD
				handleIOErrorCondition(channel, condition);
				result = -1;
#endif
			}

#ifndef CLIENT_IPC_THREAD
		}
		else
		{
			SCARD_DEBUG_ERR("Unknown channel event [%p]", channel);
		}
#endif

		SCARD_END();

		return result;
	}

} /* namespace open_mobile_api */
