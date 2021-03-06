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
#include "PKCS15DODF.h"
#include "SimpleTLV.h"

namespace smartcard_service_api
{
//	PKCS15DODF::PKCS15DODF():PKCS15Object()
//	{
//
//	}

	PKCS15DODF::PKCS15DODF(unsigned int fid, Channel *channel):PKCS15Object(channel)
	{
		int ret = 0;

		if ((ret = select(fid)) == 0)
		{
			ByteArray dodfData, extra;

			SCARD_DEBUG("response : %s", selectResponse.toString());

			if ((ret = readBinary(0, 0, getFCP()->getFileSize(), dodfData)) == 0)
			{
				SCARD_DEBUG("odfData : %s", dodfData.toString());

				parseData(dodfData);
			}
			else
			{
				SCARD_DEBUG_ERR("readBinary failed, [%d]", ret);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select failed, [%d]", ret);
		}
	}

	PKCS15DODF::PKCS15DODF(ByteArray path, Channel *channel):PKCS15Object(channel)
	{
		int ret = 0;

		if ((ret = select(path)) == 0)
		{
			ByteArray dodfData, extra;

			SCARD_DEBUG("response : %s", selectResponse.toString());

			if ((ret = readBinary(0, 0, getFCP()->getFileSize(), dodfData)) == 0)
			{
				SCARD_DEBUG("dodfData : %s", dodfData.toString());

				parseData(dodfData);
			}
			else
			{
				SCARD_DEBUG_ERR("readBinary failed, [%d]", ret);
			}
		}
		else
		{
			SCARD_DEBUG_ERR("select failed, [%d]", ret);
		}
	}

	PKCS15DODF::~PKCS15DODF()
	{
	}

	bool PKCS15DODF::parseData(ByteArray data)
	{
		bool result = false;
		SimpleTLV tlv(data);

		while (tlv.decodeTLV())
		{
			switch (tlv.getTag())
			{
			case (unsigned int)0xA1 : /* CHOICE 1 : OidDO */
				{
					PKCS15OID oid(tlv.getValue());

					SCARD_DEBUG("OID DataObject");

					pair<ByteArray, PKCS15OID> newPair(oid.getOID(), oid);
					mapOID.insert(newPair);
				}
				break;

			default :
				SCARD_DEBUG("Unknown tlv : t [%X], l [%d], v %s", tlv.getTag(), tlv.getLength(), tlv.getValue().toString());
				break;
			}
		}

		SCARD_DEBUG("dataList.size() = %d", mapOID.size());

		return result;
	}

	int PKCS15DODF::searchOID(ByteArray oid, ByteArray &data)
	{
		int result = -1;
		map<ByteArray, PKCS15OID>::iterator item;

		item = mapOID.find(oid);
		if (item != mapOID.end())
		{
			data = item->second.getPath();
			result = 0;
		}

		return result;
	}

} /* namespace smartcard_service_api */
