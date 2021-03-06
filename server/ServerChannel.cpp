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

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ServerChannel.h"
#include "APDUHelper.h"

namespace smartcard_service_api
{
	ServerChannel::ServerChannel(ServerSession *session, void *caller, int channelNum, Terminal *terminal):Channel(session)
	{
		this->terminal = terminal;
		this->caller = caller;
		this->channelNum = channelNum;
	}

	ServerChannel::~ServerChannel()
	{
		if (isClosed() == false)
		{
			closeSync();
		}
	}

	void ServerChannel::closeSync()
	{
		ByteArray command, result;
		APDUHelper apdu;
		int rv;

		if (isBasicChannel() == false)
		{
			/* close channel */
			command = apdu.generateAPDU(APDUHelper::COMMAND_CLOSE_LOGICAL_CHANNEL, channelNum, ByteArray::EMPTY);
			rv = terminal->transmitSync(command, result);

			if (rv == 0 && result.getLength() >= 2)
			{
				ResponseHelper resp(result);

				if (resp.getStatus() == 0)
				{
					SCARD_DEBUG("close success");
				}
				else
				{
					SCARD_DEBUG_ERR("status word [%d][ 0x%02X 0x%02X ]", resp.getStatus(), result[result.getLength() - 2], result[result.getLength() - 1]);
				}
			}
			else
			{
				SCARD_DEBUG_ERR("select apdu is failed, rv [%d], length [%d]", rv, result.getLength());
			}
		}

		channelNum = -1;
	}

	int ServerChannel::getChannelNumber()
	{
		return channelNum;
	}

	int ServerChannel::transmitSync(ByteArray command, ByteArray &result)
	{
		APDUCommand helper;

		if (session != NULL) /* admin channel */
		{
			helper.setCommand(command);

			/* filter command */
			if ((helper.getINS() == APDUCommand::INS_SELECT_FILE && helper.getP1() == APDUCommand::P1_SELECT_BY_DF_NAME) ||
				(helper.getINS() == APDUCommand::INS_MANAGE_CHANNEL))
			{
				return -4; /* security reason */
			}

			/* insert channel ID */
			helper.setChannel(0, channelNum);

			helper.getBuffer(command);
		}

		SCARD_DEBUG("command [%d] : %s", command.getLength(), command.toString());

		return terminal->transmitSync(command, result);
	}

} /* namespace smartcard_service_api */
