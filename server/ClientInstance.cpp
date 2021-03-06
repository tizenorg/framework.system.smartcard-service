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
#include <pthread.h>

/* SLP library header */

/* local header */
#include "Debug.h"
#include "ClientInstance.h"
#include "ServerResource.h"
#include "SignatureHelper.h"

namespace smartcard_service_api
{
	void ClientInstance::setPID(int pid)
	{
		this->pid = pid;

#if 0
		if (pid > 0)
		{
			certHash = SignatureHelper::getCertificationHash(pid);
		}
#endif
	}

	bool ClientInstance::createService(unsigned int context)
	{
		bool result = false;

		if (getService(context) == NULL)
		{
			ServiceInstance *instance = new ServiceInstance(this, context);
			if (instance != NULL)
			{
				mapServices.insert(make_pair(context, instance));
				result = true;
			}
			else
			{
				SCARD_DEBUG_ERR("alloc failed");
			}
		}
		else
		{
			SCARD_DEBUG_ERR("service already exist [%d]", context);
		}

		return result;
	}

	ServiceInstance *ClientInstance::getService(unsigned int context)
	{
		ServiceInstance *result = NULL;
		map<unsigned int, ServiceInstance *>::iterator item;

		if ((item = mapServices.find(context)) != mapServices.end())
		{
			result = item->second;
		}

		return result;
	}

	void ClientInstance::removeService(unsigned int context)
	{
		map<unsigned int, ServiceInstance *>::iterator item;

		if ((item = mapServices.find(context)) != mapServices.end())
		{
			delete item->second;
			mapServices.erase(item);
		}
	}

	void ClientInstance::removeServices()
	{
		map<unsigned int, ServiceInstance *>::iterator item;

		for (item = mapServices.begin(); item != mapServices.end(); item++)
		{
			delete item->second;
		}

		mapServices.clear();
	}

	bool ClientInstance::sendMessageToAllServices(int socket, Message &msg)
	{
		bool result = true;
		map<unsigned int, ServiceInstance *>::iterator item;

		for (item = mapServices.begin(); item != mapServices.end(); item++)
		{
			if (ServerIPC::getInstance()->sendMessage(socket, &msg) == false)
				result = false;
		}

		return result;
	}

} /* namespace smartcard_service_api */
