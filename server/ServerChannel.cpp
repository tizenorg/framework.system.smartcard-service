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

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ServerChannel.h"
#include "APDUHelper.h"

namespace smartcard_service_api
{
	ServerChannel::ServerChannel(ServerSession *session, void *caller,
		int channelNum, Terminal *terminal) :
		Channel(session), terminal(terminal), caller(caller),
		privilege(true)
	{
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
		throw(ErrorIO &, ErrorIllegalState &)
	{
		ByteArray command, result;
		APDUHelper apdu;
		int rv;

		if (isClosed() == false && isBasicChannel() == false)
		{
			/* close channel */
			command = apdu.generateAPDU(APDUHelper::COMMAND_CLOSE_LOGICAL_CHANNEL, channelNum, ByteArray::EMPTY);

			_DBG("command [%d] : %s", command.size(), command.toString().c_str());

			rv = terminal->transmitSync(command, result);

			if (rv == 0 && result.size() >= 2)
			{
				ResponseHelper resp(result);

				if (resp.getStatus() >= 0)
				{
					_DBG("close success");
				}
				else
				{
					_ERR("status word [ %02X %02X ]", resp.getSW1(), resp.getSW2());
				}
			}
			else
			{
				_ERR("select apdu is failed, rv [%d], length [%d]", rv, result.size());
			}

			channelNum = -1;
		}
	}

	int ServerChannel::transmitSync(const ByteArray &command, ByteArray &result)
		throw(ErrorIO &, ErrorIllegalState &, ErrorIllegalParameter &, ErrorSecurity &)
	{
		int ret = -1;
		APDUCommand helper;
		ByteArray cmd;

		if (isClosed() == true)
		{
			return ret;
		}

		helper.setCommand(command);

		/* filter command */
		if (privilege == false)
		{
			if ((helper.getINS() == APDUCommand::INS_SELECT_FILE &&
				helper.getP1() == APDUCommand::P1_SELECT_BY_DF_NAME) ||
				(helper.getINS() == APDUCommand::INS_MANAGE_CHANNEL))
			{
				return SCARD_ERROR_SECURITY_NOT_ALLOWED;
			}
		}

		/* TODO : insert channel ID using atr information */
		helper.setChannel(APDUCommand::CLA_CHANNEL_STANDARD, channelNum);
		helper.getBuffer(cmd);

		_DBG("command [%d] : %s", cmd.size(), cmd.toString().c_str());

		ret = terminal->transmitSync(cmd, result);

		return ret;
	}

} /* namespace smartcard_service_api */