/*
 ============================================================================
 Name                    : InternetTp1Question2_a.c
 Author                  : Leonard (Lenny) NAMOLARU
 Source des commentaires : https://curl.se/libcurl/c/libcurl.html
 gcc InternetTp1Question2_a.c -lcurl
 ============================================================================
 */

#include <stdio.h> // snprintf(), fprintf()
#include <stdlib.h> // exit(), EXIT_SUCCESS, EXIT_FAILURE, realloc(), malloc(), free()
#include <string.h> // memcpy(), strncpy(), strlen()
#include <curl/curl.h> // curl_easy_init(), curl_easy_setopt(), curl_easy_perform(), curl_easy_cleanup()

#define MAX_COUNT 50
#define	HOST_URL "https://localhost:8443/chat/"

typedef struct string {
	char* str;
	size_t length;
} string ;

static size_t get_protocol_response(void* protocol_response, size_t size, size_t protocol_response_length, void* memory_struct) {
	size_t real_size = size * protocol_response_length;

	string* string_struct = (string *) memory_struct;

	// void * realloc (void *ptr, size_t newsize)
	char* ptr = realloc(string_struct->str, string_struct->length + real_size + 1);
	if (ptr == NULL) {
		fprintf(stderr, "Out of memory ! \n");
		exit(EXIT_FAILURE);
	}

	string_struct->str = ptr;

	// void * memcpy (void *restrict to, const void *restrict from, size_t size)
	memcpy(&(string_struct->str[string_struct->length]), protocol_response, real_size);
	string_struct->length += real_size;

	string_struct->str[string_struct->length] = 0;

	return real_size;
}

int main(int argc, char* argv[]) {
	CURL *curl;
	CURLcode res;

	// Array partially initialized => elements that are not initialized receive 0.
	char* id_list[MAX_COUNT] = {0}; // Initialize array to 0
	int count = MAX_COUNT;

	// CURL *curl_easy_init(); - Start a libcurl easy session
	// This function must be the first function to call,
	// and it returns a CURL easy handle that you must use as input
	// to other functions in the easy interface.
	// This call MUST have a corresponding call to curl_easy_cleanup
	// when the operation is complete.
	curl = curl_easy_init();

	// If this function returns NULL,
	// something went wrong and you cannot use the other curl functions.
	if (curl != NULL) {
		// CURLcode curl_easy_setopt(CURL *handle, CURLoption option, parameter);
		// Set options for a curl easy handle. curl_easy_setopt is used to tell libcurl how to behave.
		// The handle is the return code from a curl_easy_init or curl_easy_duphandle call.
		// All options are set with an option followed by a parameter.
		// You can only set one option in each function call.
		// A typical application uses many curl_easy_setopt calls in the setup phase.

		// Return value : CURLE_OK (zero) means that the option was set properly,
		// non-zero means an error occurred as <curl/curl.h> defines.

		// CURLcode curl_easy_setopt(CURL *handle, CURLOPT_URL, char *URL);
		// CURLOPT_URL - URL for this transfer
		curl_easy_setopt(curl, CURLOPT_URL, HOST_URL);

		// CURLcode curl_easy_setopt(CURL *handle, CURLOPT_SSL_VERIFYPEER, long verify);
		// CURLOPT_SSL_VERIFYPEER - verify the peer's SSL certificate
		// This option determines whether curl verifies the authenticity of the peer's certificate.
		// A value of 1 means curl verifies; 0 (zero) means it does not.

		// This is a code for pedagogical purposes !
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		string messeges_id = {0};
		string messege_body = {0};

		// CURLcode curl_easy_setopt(CURL *handle, CURLOPT_WRITEFUNCTION, write_callback);
		// Send all data to this function
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_protocol_response);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&messeges_id);

		// Perform the request, res will get the return code.
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed : %s \n", curl_easy_strerror(res));
			exit (EXIT_FAILURE);
		}

		printf("HTTP GET REQUEST TO : %s \n", HOST_URL);
		printf("%s \n", messeges_id.str);

		int i;
		int id_end = messeges_id.length - 2, id_start;
		for (i = messeges_id.length - 2 ; i >= 0  && count > 0 ; i--) {

			if(messeges_id.str[i] == '\n' || i == 0) {
				if (i == 0)
					id_start = i;
				else // messeges_id.str[i] == '\n'
					id_start = i + 1;

				int id_len = (id_end - id_start) + 1;

				if(id_len > 0) {
					// void * malloc (size_t size)
					char* msg_id = (char*) malloc(sizeof(char) * (id_len + 1));
					if(msg_id == NULL) {
						fprintf(stderr, "Out of memory ! \n");
						exit(EXIT_FAILURE);
					} // if

					// char * strncpy (char *restrict to, const char *restrict from, size_t size)
					strncpy(msg_id, messeges_id.str + id_start, id_len);
					msg_id[id_len] = '\0';
					id_list[count - 1] = msg_id;

					count--;
				} // if

				id_end = i - 1;

			} // if
		} // for

		free(messeges_id.str); // void free (void *ptr)

		// Options set with this function call are valid for all forthcoming transfers performed using this handle.
		// The options are not in any way reset between transfers, so if you want subsequent transfers with different options,
		// you must change them between the transfers.
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &messege_body);

		count = 0;
		for(i = 0 ; i < MAX_COUNT ; i++) {
			if(id_list[i] != 0) {
				char* msg_id = id_list[i];
				int id_len = strlen(msg_id); // size_t strlen (const char *s)
				printf("Id of messege %d : %s \n", ++count, msg_id);

				int url_length = (strlen(HOST_URL) + id_len + 1); // +1 for '\0'
				char* url = (char *) malloc(sizeof(char) * url_length);


				// int snprintf (char *s, size_t size, const char *template, ...)
				// The snprintf function is similar to sprintf,
				// except that the size argument specifies the maximum number of characters to produce.
				// The trailing null character is counted towards this limit,
				// so you should allocate at least size characters for the string s. (Source : Doc via Eclipse)
				snprintf(url, url_length, "%s%s", HOST_URL, msg_id);

				printf("HTTP GET REQUEST TO : %s \n", url);
				curl_easy_setopt(curl, CURLOPT_URL, url);

				// Perform the request, res will get the return code.
				res = curl_easy_perform(curl);
				if (res != CURLE_OK) {
					fprintf(stderr, "curl_easy_perform() failed : %s \n", curl_easy_strerror(res));
					exit (EXIT_FAILURE);
				}
				printf("RESULT : %s \n", messege_body.str);
				printf("\n");
				messege_body.length = 0;
				messege_body.str = 0;

				// void free (void *ptr)
				free(messege_body.str);
				free(msg_id);
				free(url);
			} // if
		} // for

		curl_easy_cleanup(curl);

	} else {
		// int fprintf (FILE *stream, const char *template, ...)
		fprintf(stderr, "Something went wrong \n");
		exit (EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
