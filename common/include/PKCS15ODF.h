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

#ifndef PKCS15ODF_H_
#define PKCS15ODF_H_

/* standard library header */
#include <map>

/* SLP library header */

/* local header */
#include "FileObject.h"
#include "PKCS15CDF.h"
#include "PKCS15DODF.h"

using namespace std;

namespace smartcard_service_api
{
	class PKCS15ODF : public PKCS15Object
	{
	private:
		PKCS15CDF *cdf;
		PKCS15DODF *dodf;

		bool parseData(const ByteArray &data);

	public:
		PKCS15ODF(Channel *channel);
		PKCS15ODF(Channel *channel, const ByteArray &selectResponse);
		~PKCS15ODF();

		int getPuKDFPath(ByteArray &path);
		int getPrKDFPath(ByteArray &path);
		int getAODFPath(ByteArray &path);
		int getCDFPath(ByteArray &path);
		int getDODFPath(ByteArray &path);

		PKCS15CDF *getCDF();
		PKCS15DODF *getDODF();
	};

} /* namespace smartcard_service_api */
#endif /* PKCS15ODF_H_ */