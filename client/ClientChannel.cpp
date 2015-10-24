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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ClientChannel.h"
#include "ReaderHelper.h"
#include "APDUHelper.h"
#include "ClientGDBus.h"

#ifndef EXTERN_API
#define EXTERN_API __attribute__((visibility("default")))
#endif

namespace smartcard_service_api
{
	ClientChannel::ClientChannel(void *context, Session *session,
		int channelNum, const ByteArray &selectResponse, void *handle)
		: Channel(session)
	{
		this->channelNum = -1;
		this->handle = NULL;
		this->context = NULL;

		if (handle == NULL)
		{
			_ERR("ClientIPC::getInstance() failed");

			return;
		}

		this->channelNum = channelNum;
		this->handle = handle;
		this->selectResponse = selectResponse;
		this->context = context;

		/* init default context */
		GError *error = NULL;

		proxy = smartcard_service_channel_proxy_new_for_bus_sync(
			G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
			"org.tizen.SmartcardService",
			"/org/tizen/SmartcardService/Channel",
			NULL, &error);
		if (proxy == NULL)
		{
			_ERR("Can not create proxy : %s", error->message);
			g_error_free(error);
			return;
		}
	}

	ClientChannel::~ClientChannel()
	{
		closeSync();
	}

	void ClientChannel::channel_transmit_cb(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
	{
		CallbackParam *param = (CallbackParam *)user_data;
		transmitCallback callback;
		gint result;
		GVariant *var_response;
		GError *error = NULL;
		ByteArray response;

		_INFO("MSG_REQUEST_TRANSMIT");

		if (param == NULL) {
			_ERR("null parameter!!!");
			return;
		}

		callback = (transmitCallback)param->callback;

		if (smartcard_service_channel_call_transmit_finish(
			SMARTCARD_SERVICE_CHANNEL(source_object),
			&result, &var_response, res, &error) == true) {
			if (result == SCARD_ERROR_OK) {
				GDBusHelper::convertVariantToByteArray(var_response, response);
			} else {
				_ERR("smartcard_service_channel_call_transmit failed, [%d]", result);
			}
		} else {
			_ERR("smartcard_service_channel_call_transmit failed, [%s]", error->message);
			g_error_free(error);

			result = SCARD_ERROR_IPC_FAILED;
		}

		if (callback != NULL) {
			callback(response.getBuffer(),
				response.size(),
				result, param->user_param);
		}

		delete param;
	}

	void ClientChannel::channel_close_cb(GObject *source_object,
		GAsyncResult *res, gpointer user_data)
	{
		CallbackParam *param = (CallbackParam *)user_data;
		ClientChannel *channel;
		closeChannelCallback callback;
		gint result;
		GError *error = NULL;

		_INFO("MSG_REQUEST_CLOSE_CHANNEL");

		if (param == NULL) {
			_ERR("null parameter!!!");
			return;
		}

		channel = (ClientChannel *)param->instance;
		callback = (closeChannelCallback)param->callback;

		if (smartcard_service_channel_call_close_channel_finish(
			SMARTCARD_SERVICE_CHANNEL(source_object),
			&result, res, &error) == true) {
			if (result == SCARD_ERROR_OK) {
				channel->channelNum = -1;
			} else {
				_ERR("smartcard_service_channel_call_close_channel failed, [%d]", result);
			}
		} else {
			_ERR("smartcard_service_channel_call_close_channel failed, [%s]", error->message);
			g_error_free(error);

			result = SCARD_ERROR_IPC_FAILED;
		}

		if (callback != NULL) {
			callback(result, param->user_param);
		}

		delete param;
	}

	void ClientChannel::closeSync()
		throw(ExceptionBase &, ErrorIO &, ErrorSecurity &,
			ErrorIllegalState &, ErrorIllegalParameter &)
	{
		if (isClosed() == false)
		{
			if (getSession()->getReader()->isSecureElementPresent() == true)
			{
				gint ret;
				GError *error = NULL;


				if (proxy == NULL) {
					_ERR("dbus proxy is not initialized yet");
					throw ErrorIllegalState(SCARD_ERROR_NOT_INITIALIZED);
				}

				if (smartcard_service_channel_call_close_channel_sync(
					(SmartcardServiceChannel *)proxy,
					GPOINTER_TO_UINT(context),
					GPOINTER_TO_UINT(handle),
					&ret, NULL, &error) == true) {
					if (ret == SCARD_ERROR_OK) {
						channelNum = -1;
					} else {
						_ERR("smartcard_service_channel_call_close_channel_sync failed, [%d]", ret);
						THROW_ERROR(ret);
					}
				} else {
					_ERR("smartcard_service_channel_call_close_channel_sync failed, [%s]", error->message);
					g_error_free(error);

					throw ErrorIO(SCARD_ERROR_IPC_FAILED);
				}
			}
			else
			{
				_INFO("unavailable channel");
			}
		}
	}

	int ClientChannel::close(closeChannelCallback callback, void *userParam)
	{
		int result = SCARD_ERROR_OK;

		if (isClosed() == false)
		{
			if (getSession()->getReader()->isSecureElementPresent() == true)
			{
				CallbackParam *param = new CallbackParam();

				param->instance = this;
				param->callback = (void *)callback;
				param->user_param = userParam;

				smartcard_service_channel_call_close_channel(
					(SmartcardServiceChannel *)proxy,
					GPOINTER_TO_UINT(context),
					GPOINTER_TO_UINT(handle), NULL,
					&ClientChannel::channel_close_cb, param);
			}
			else
			{
				_ERR("unavailable channel");
				result = SCARD_ERROR_ILLEGAL_STATE;
			}
		}

		return result;
	}

	int ClientChannel::transmitSync(const ByteArray &command, ByteArray &result)
		throw(ExceptionBase &, ErrorIO &, ErrorIllegalState &,
			ErrorIllegalParameter &, ErrorSecurity &)
	{
		int rv = SCARD_ERROR_OK;

		if (getSession()->getReader()->isSecureElementPresent() == true)
		{
			GVariant *var_command = NULL, *var_response = NULL;
			GError *error = NULL;

			var_command = GDBusHelper::convertByteArrayToVariant(command);

			if (smartcard_service_channel_call_transmit_sync(
				(SmartcardServiceChannel *)proxy,
				GPOINTER_TO_UINT(context),
				GPOINTER_TO_UINT(handle),
				var_command, &rv, &var_response,
				NULL, &error) == true) {

				if (rv == SCARD_ERROR_OK) {
					GDBusHelper::convertVariantToByteArray(var_response, transmitResponse);
					result = transmitResponse;
				} else {
					_ERR("smartcard_service_session_call_transmit failed, [%d]", rv);
					THROW_ERROR(rv);
				}
			} else {
				_ERR("smartcard_service_session_call_transmit failed, [%s]", error->message);
				g_error_free(error);

				throw ErrorIO(SCARD_ERROR_IPC_FAILED);
			}
		}
		else
		{
			_ERR("unavailable channel");
			throw ErrorIllegalState(SCARD_ERROR_UNAVAILABLE);
		}

		return rv;
	}

	int ClientChannel::transmit(const ByteArray &command, transmitCallback callback, void *userParam)
	{
		int result;

		if (getSession()->getReader()->isSecureElementPresent() == true)
		{
			GVariant *var_command;
			CallbackParam *param = new CallbackParam();

			param->instance = this;
			param->callback = (void *)callback;
			param->user_param = userParam;

			var_command = GDBusHelper::convertByteArrayToVariant(command);

			smartcard_service_channel_call_transmit(
				(SmartcardServiceChannel *)proxy,
				GPOINTER_TO_UINT(context),
				GPOINTER_TO_UINT(handle),
				var_command, NULL,
				&ClientChannel::channel_transmit_cb, param);

			result = SCARD_ERROR_OK;
		}
		else
		{
			_ERR("unavailable channel");
			result = SCARD_ERROR_ILLEGAL_STATE;
		}

		return result;
	}
} /* namespace smartcard_service_api */

/* export C API */
#define CHANNEL_EXTERN_BEGIN \
	if (handle != NULL) \
	{ \
		ClientChannel *channel = (ClientChannel *)handle;

#define CHANNEL_EXTERN_END \
	} \
	else \
	{ \
		_ERR("Invalid param"); \
	}

using namespace smartcard_service_api;

EXTERN_API int channel_close_sync(channel_h handle)
{
	int result = SCARD_ERROR_OK;

	CHANNEL_EXTERN_BEGIN;

	try
	{
		channel->closeSync();
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
	}

	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API int channel_transmit_sync(channel_h handle, unsigned char *command,
	unsigned int cmd_len, unsigned char **response, unsigned int *resp_len)
{
	int result = SCARD_ERROR_OK;

	if (command == NULL || cmd_len == 0 || response == NULL || resp_len == NULL)
		return SCARD_ERROR_UNKNOWN;

	CHANNEL_EXTERN_BEGIN;

	try
	{
		ByteArray temp, resp;

		temp.assign(command, cmd_len);
		channel->transmitSync(temp, resp);

		if (resp.size() > 0)
		{
			*response = (unsigned char *)calloc(resp.size(), sizeof(unsigned char));
			*resp_len = resp.size();
			memcpy(*response, resp.getBuffer(), *resp_len);
		}
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
		*resp_len = 0;
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
		*resp_len = 0;
	}

	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API int channel_is_basic_channel(channel_h handle, bool* is_basic_channel)
{
	int result = SCARD_ERROR_OK;

	CHANNEL_EXTERN_BEGIN;

	try
	{
		*is_basic_channel = channel->isBasicChannel();
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
	}

	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API int channel_is_closed(channel_h handle, bool* is_closed)
{
	int result = SCARD_ERROR_OK;

	CHANNEL_EXTERN_BEGIN;

	try
	{
		*is_closed = channel->isClosed();
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
	}

	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API int channel_get_select_response(channel_h handle,
	unsigned char **buffer, size_t *length)
{
	int result = SCARD_ERROR_OK;

	CHANNEL_EXTERN_BEGIN;

	try
	{
		ByteArray response = channel->getSelectResponse();
		uint8_t* get_buffer = response.getBuffer();

		*length = response.size();

		if (*length > 0)
		{
			*buffer = (unsigned char *)calloc(*length, sizeof(unsigned char));
			if(*buffer == NULL || get_buffer == NULL)
			{
				*length = 0;
				return SCARD_ERROR_NOT_ENOUGH_RESOURCE;
			}

			memcpy(*buffer, get_buffer, *length);
		}
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
		*length = 0;
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
		*length = 0;
	}

	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API int channel_get_transmit_response(channel_h handle,
	unsigned char **buffer, size_t *length)
{
	int result = SCARD_ERROR_OK;

	CHANNEL_EXTERN_BEGIN;

	try
	{
		ByteArray response;

		response = channel->getTransmitResponse();

		*length = response.size();

		if (*length > 0)
		{
			*buffer = (unsigned char *)calloc(*length, sizeof(unsigned char));
			memcpy(*buffer, response.getBuffer(), *length);
		}
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
		*length = 0;
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
		*length = 0;
	}

	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API int channel_get_session(channel_h handle, int *session_handle)
{
	int result = SCARD_ERROR_OK;
	session_h session = NULL;

	CHANNEL_EXTERN_BEGIN;

	try
	{
		session = channel->getSession();
		*session_handle = (int)session;
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
	}

	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API int channel_select_next(channel_h handle, bool *pSuccess)
{
	int result = SCARD_ERROR_OK;

	CHANNEL_EXTERN_BEGIN;

	try
	{
		*pSuccess = channel->selectNext();
	}
	catch (ExceptionBase &e)
	{
		_ERR("Error occur : %s\n", e.what());
		result = e.getErrorCode();
	}
	catch (...)
	{
		_ERR("Error occur : unknown error\n");
		result = SCARD_ERROR_UNKNOWN;
	}

	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API unsigned int channel_get_select_response_length(channel_h handle)
{
	unsigned int result = 0;

	CHANNEL_EXTERN_BEGIN;
	result = channel->getSelectResponse().size();
	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API int channel_close(channel_h handle, channel_close_cb callback, void *userParam)
{
	int result = -1;

	CHANNEL_EXTERN_BEGIN;
	result = channel->close((closeChannelCallback)callback, userParam);
	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API int channel_transmit(channel_h handle, unsigned char *command,
	unsigned int length, channel_transmit_cb callback, void *userParam)
{
	int result = -1;

	CHANNEL_EXTERN_BEGIN;
	ByteArray temp;

	temp.assign(command, length);
	result = channel->transmit(temp, (transmitCallback)callback, userParam);
	CHANNEL_EXTERN_END;

	return result;
}

EXTERN_API void channel_destroy_instance(channel_h handle)
{
	/* do nothing */
}
