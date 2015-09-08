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

#ifndef SESERVICE_H_
#define SESERVICE_H_

/* standard library header */
#include <glib.h>
#include <gio/gio.h>

/* SLP library header */

/* local header */
#include "smartcard-types.h"
#ifdef __cplusplus
#include "SEServiceListener.h"
#include "SEServiceHelper.h"
#endif /* __cplusplus */

#ifdef __cplusplus
using namespace std;

namespace smartcard_service_api
{
	typedef void (*serviceConnected)(SEServiceHelper *service, void *context);

	class SEService : public SEServiceHelper
	{
	private:
		unsigned int handle;
		void *context;
		serviceConnected handler;
		SEServiceListener *listener;
		void *proxy;
		const char *version;

		SEService();

		void addReader(unsigned int handle, const char *name);
		bool parseReaderInformation(unsigned int count, const ByteArray &data);
		bool parseReaderInformation(GVariant *variant);

		bool _initialize()
			throw(ErrorIO &);
		bool initialize(void *context, serviceConnected handler)
			throw(ErrorIO &, ErrorIllegalParameter &);
		bool initialize(void *context, SEServiceListener *listener)
			throw(ErrorIO &, ErrorIllegalParameter &);
		int _initialize_sync()
			throw(ErrorIO &, ExceptionBase &);
		bool initializeSync(void *context)
			throw(ErrorIO &, ErrorIllegalParameter &, ExceptionBase &);

		static void reader_inserted(GObject *source_object,
			guint reader_id, gchar *reader_name,
			gpointer user_data);
		static void reader_removed(GObject *source_object,
			guint reader_id, gchar *reader_name,
			gpointer user_data);
		static void se_service_shutdown_cb(GObject *source_object,
			GAsyncResult *res, gpointer user_data);
		static void se_service_cb(GObject *source_object,
			GAsyncResult *res, gpointer user_data);

	public:
		SEService(void *user_data, serviceConnected handler)
			throw(ErrorIO &, ErrorIllegalParameter &);
		SEService(void *user_data, SEServiceListener *listener)
			throw(ErrorIO &, ErrorIllegalParameter &);
		SEService(void *user_data)
			throw(ErrorIO &, ErrorIllegalParameter &, ExceptionBase &);
		~SEService();

		static SEService *createInstance(void *user_data, SEServiceListener *listener)
			throw(ErrorIO &, ErrorIllegalParameter &);
		static SEService *createInstance(void *user_data, serviceConnected handler)
			throw(ErrorIO &, ErrorIllegalParameter &);

		void shutdown();
		void shutdownSync();
		const char *getVersion() const { return version; }
	};
} /* namespace smartcard_service_api */
#endif /* __cplusplus */

/* export C API */
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

se_service_h se_service_create_instance(void *user_data,
	se_service_connected_cb callback);
se_service_h se_service_create_instance_with_event_callback(void *user_data,
	se_service_connected_cb connected, se_service_event_cb event,
	se_sesrvice_error_cb error);

se_service_h se_service_create_instance_sync(void *user_data, int* result);
int se_service_get_version(se_service_h handle, char **version_str);
int se_service_get_readers_count(se_service_h handle);
int se_service_get_readers(se_service_h handle, int **readers, int *count);

bool se_service_is_connected(se_service_h handle);
void se_service_shutdown(se_service_h handle);
int se_service_destroy_instance(se_service_h handle);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SESERVICE_H_ */