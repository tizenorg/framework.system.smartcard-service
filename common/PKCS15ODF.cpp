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
#include "PKCS15ODF.h"
#include "SimpleTLV.h"
#include "NumberStream.h"

namespace smartcard_service_api
{
	static const unsigned int ODF_FID = 0x3150;

	static const unsigned int TAG_DODF = (unsigned int)0xA7;
	static const unsigned int TAG_CDF = (unsigned int)0xA5;

	PKCS15ODF::PKCS15ODF(Channel *channel) :
		PKCS15Object(channel), cdf(NULL), dodf(NULL)
	{
		int ret = 0;

		if ((ret = select(ODF_FID)) >= SCARD_ERROR_OK)
		{
			ByteArray odfData, extra;

			_DBG("response : %s", selectResponse.toString().c_str());

			if ((ret = readBinaryAll(0, odfData)) == 0)
			{
				_DBG("odfData : %s", odfData.toString().c_str());

				parseData(odfData);
			}
			else
			{
				_ERR("readBinary failed, [%d]", ret);
			}
		}
		else
		{
			_ERR("select failed, [%d]", ret);
		}
	}

	PKCS15ODF::PKCS15ODF(Channel *channel, const ByteArray &selectResponse) :
		PKCS15Object(channel, selectResponse), dodf(NULL)
	{
		int ret = 0;
		ByteArray odfData;

		if ((ret = readBinaryAll(0, odfData)) == 0)
		{
			_DBG("odfData : %s", odfData.toString().c_str());

			parseData(odfData);
		}
		else
		{
			_ERR("readBinary failed, [%d]", ret);
		}
	}

	PKCS15ODF::~PKCS15ODF()
	{
		if (dodf != NULL)
		{
			delete dodf;
			dodf = NULL;
		}
	}

	bool PKCS15ODF::parseData(const ByteArray &data)
	{
		bool result = false;
		SimpleTLV tlv(data);

		while (tlv.decodeTLV())
		{
			switch (tlv.getTag())
			{
			case TAG_DODF :
				{
					ByteArray dodf;

					_DBG("TAG_DODF");

					dodf = PKCS15Object::getOctetStream(tlv.getValue());

					_DBG("path : %s", dodf.toString().c_str());

					pair<unsigned int, ByteArray> newPair(tlv.getTag(), dodf);
					dataList.insert(newPair);
				}
				break;

			case TAG_CDF :
				{
					ByteArray tokeninfo;

					_DBG("TAG_CDF");

					tokeninfo = PKCS15Object::getOctetStream(tlv.getValue());

					_DBG("path : %s", tokeninfo.toString().c_str());

					pair<unsigned int, ByteArray> newPair(tlv.getTag(), tokeninfo);
					dataList.insert(newPair);
				}
				break;

			default :
				_DBG("Unknown tlv : t [%X], l [%d], v %s",
					tlv.getTag(), tlv.size(), tlv.getValue().toString().c_str());
				break;
			}

		}

		_INFO("dataList.size() = %d", dataList.size());

		return result;
	}

	PKCS15DODF *PKCS15ODF::getDODF()
	{
		map<unsigned int, ByteArray>::iterator item;

		if (dodf == NULL)
		{
			item = dataList.find(TAG_DODF);
			if (item != dataList.end())
			{
				NumberStream num(item->second);
				unsigned int fid = num.getLittleEndianNumber();

				_DBG("dodf fid [%X]", fid);

				dodf = new PKCS15DODF(fid, channel);
				if (dodf != NULL && dodf->isClosed() == true)
				{
					_ERR("failed to open DODF");

					delete dodf;
					dodf = NULL;
				}
			}
			else
			{
				_ERR("[%02X] is not found. total [%d]", TAG_DODF, dataList.size());
			}
		}

		_DBG("dodf [%p]", dodf);

		return dodf;
	}

	PKCS15CDF *PKCS15ODF::getCDF()
	{
		map<unsigned int, ByteArray>::iterator item;

		if (cdf == NULL)
		{
			item = dataList.find(TAG_CDF);
			if (item != dataList.end())
			{
				NumberStream num(item->second);
				unsigned int fid = num.getLittleEndianNumber();

				_DBG("cdf fid [%X]", fid);

				cdf = new PKCS15CDF(fid, channel);
				if (cdf != NULL && cdf->isClosed() == true)
				{
					_ERR("failed to open CDF");

					delete cdf;
					cdf = NULL;
				}
			}
			else
			{
				_ERR("[%02X] is not found. total [%d]", TAG_CDF, dataList.size());
			}
		}

		_DBG("cdf [%p]", cdf);

		return cdf;
	}
} /* namespace smartcard_service_api */
