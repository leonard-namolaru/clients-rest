/*
 ============================================================================
 Name                    : InternetTp1Question4.c
 Author                  : Leonard (Lenny) NAMOLARU
 Source des commentaires : https://curl.se/libcurl/c/libcurl.html
 gcc InternetTp1Question4.c -lcurl
 ============================================================================
 */

#include <stdio.h> // snprintf(), fprintf()
#include <stdlib.h> // exit(), EXIT_SUCCESS, EXIT_FAILURE, realloc(), malloc(), free()
#include <string.h> // memcpy(), strncpy(), strlen(), strtok(), strstr(), strncpy()
#include <curl/curl.h> // curl_easy_init(), curl_easy_setopt(), curl_easy_perform(), curl_easy_cleanup(), curl_easy_getinfo(), curl_slist_append()
#include <unistd.h> // sleep()

#define MAX_COUNT 50
#define	HOST_URL "https://localhost:8443/chat/"
#define SLEEP_INTERVAL 10
#define ID_LENGTH 32
#define	HTTP_HEADER_LAST_MODIFIED_KEY "last-modified: "
#define	HTTP_HEADER_IF_MODIFIED_SINCE_KEY "If-Modified-Since: "

typedef struct string {
	char* str;
	size_t length;
} string ;


char* get_last_modified_value(string* http_header) {
	// size_t strlen (const char *s)
	const int http_header_key_len = strlen(HTTP_HEADER_LAST_MODIFIED_KEY);
	char* http_header_value = NULL;

	// char * strstr (const char *haystack, const char *needle)
	char* http_header_value_start = strstr(http_header->str, HTTP_HEADER_LAST_MODIFIED_KEY);

	if (http_header_value_start != NULL) { // Returns a null pointer if no match was found
		http_header_value_start += http_header_key_len;
		char* http_header_value_end = strstr(http_header_value_start, "\n") - 1; // -1 => the ptr start before '\n'
		long http_header_value_len = http_header_value_end - http_header_value_start;

		http_header_value = (char *) malloc(sizeof(char)*(http_header_value_len + 1));
		if(http_header_value == NULL) {
			fprintf(stderr, "Out of memory ! \n");
			exit(EXIT_FAILURE);
		} // if

		// char * strncpy (char *restrict to, const char *restrict from, size_t size)
		strncpy(http_header_value, http_header_value_start, http_header_value_len);
	} // if


	free(http_header->str);
	http_header->str = 0;
	http_header->length = 0;

	return http_header_value;
}

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
	curl = curl_easy_init();

	// If this function returns NULL,
	// something went wrong and you cannot use the other curl functions.
	if (curl != NULL) {
		// CURLcode curl_easy_setopt(CURL *handle, CURLoption option, parameter);

		// CURLcode curl_easy_setopt(CURL *handle, CURLOPT_URL, char *URL);
		// CURLOPT_URL - URL for this transfer
		curl_easy_setopt(curl, CURLOPT_URL, HOST_URL);

		// CURLcode curl_easy_setopt(CURL *handle, CURLOPT_SSL_VERIFYPEER, long verify);
		// CURLOPT_SSL_VERIFYPEER - verify the peer's SSL certificate

		// This is a code for pedagogical purposes !
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		string messeges_id = {0};
		string messege_body = {0};

		// CURLcode curl_easy_setopt(CURL *handle, CURLOPT_WRITEFUNCTION, write_callback);
		// Send all data to this function
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_protocol_response);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&messeges_id);

		string messeges_id_header = {0};
		// CURLcode curl_easy_setopt(CURL *handle, CURLOPT_HEADERFUNCTION, header_callback)
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, get_protocol_response);
		// CURLcode curl_easy_setopt(CURL *handle, CURLOPT_HEADERDATA, void* pointer)
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, &messeges_id_header);

		// Perform the request, res will get the return code.
		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed : %s \n", curl_easy_strerror(res));
			exit (EXIT_FAILURE);
		}

		printf("HTTP GET REQUEST TO : %s \n", HOST_URL);
		printf("%s \n", messeges_id.str);
		printf("HTTP RESPONSE HEADER :\n%s \n", messeges_id_header.str);

		char* http_header_last_modified_value = get_last_modified_value(&messeges_id_header);
		if (http_header_last_modified_value != NULL) {
			printf("Http header last modified value : %s \n", http_header_last_modified_value);
		} else {
			printf("There are not messeges in the chat so no last-modified information in http header \n");
		}

		curl_off_t http_content_length; // // curl_off_t - long variable

		// CURLcode curl_easy_getinfo(CURL *curl, CURLINFO info, ... );
		// Use this function AFTER a performed transfer if you want to get transfer related data.
		// You should not free the memory returned by this function unless it is explicitly mentioned.

		// CURLcode curl_easy_getinfo(CURL *handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, curl_off_t *content_length);
		// Pass a pointer to a curl_off_t to receive the content-length of the download.
		// This is the value read from the Content-Length: field. Stores -1 if the size is not known.
		res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &http_content_length);

		// Returns CURLE_OK if the option is supported
		if(res != CURLE_OK) {
			fprintf(stderr,"The option is not supported. \n");
			exit (EXIT_FAILURE);
		}

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

		curl_off_t last_request_length = http_content_length;

		while(1) {
			// unsigned int sleep (unsigned int seconds)
			sleep(SLEEP_INTERVAL);

			messeges_id.str = 0;
			messeges_id.length = 0;

			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&messeges_id);
			curl_easy_setopt(curl, CURLOPT_URL, HOST_URL);

			struct curl_slist *list = NULL;
			char* if_modified_since = NULL;
			if (http_header_last_modified_value != NULL) {
				int if_modified_since_len = strlen(HTTP_HEADER_IF_MODIFIED_SINCE_KEY) + strlen(http_header_last_modified_value);

				if_modified_since = (char*) malloc(sizeof(char)*(if_modified_since_len + 1));
				if (if_modified_since == NULL) {
					fprintf(stderr, "Out of memory ! \n");
					exit(EXIT_FAILURE);
				}

				// int snprintf (char *s, size_t size, const char *template, ...)
				snprintf(if_modified_since, if_modified_since_len + 1, "%s%s", HTTP_HEADER_IF_MODIFIED_SINCE_KEY, http_header_last_modified_value); // +1 for '\0'
				printf("%s \n", if_modified_since);

				list = curl_slist_append(list, if_modified_since);
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
			}

			// Perform the request, res will get the return code.
			res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
				fprintf(stderr, "curl_easy_perform() failed : %s \n", curl_easy_strerror(res));
				exit (EXIT_FAILURE);
			}

			if (http_header_last_modified_value != NULL) {
				curl_slist_free_all(list); // free the list
				free(if_modified_since);
			}

			// CURLcode curl_easy_getinfo(CURL *handle, CURLINFO_RESPONSE_CODE, long *codep);
			long response_code;
			res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
			if(res != CURLE_OK) {
				fprintf(stderr,"The option is not supported. \n");
				exit (EXIT_FAILURE);
			}

			printf("HTTP RESPONSE STATUS : %ld \n", response_code);

			curl_off_t check = 1;
			if(http_header_last_modified_value == NULL) { // If there are no msg in the chat
				res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &check);
				// Returns CURLE_OK if the option is supported
				if(res != CURLE_OK) {
					fprintf(stderr,"The option is not supported. \n");
					exit (EXIT_FAILURE);
				}
			}

			if (response_code != 304 && check != 0) { // 304 Not Modified

				res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &http_content_length);
				// Returns CURLE_OK if the option is supported
				if(res != CURLE_OK) {
					fprintf(stderr,"The option is not supported. \n");
					exit (EXIT_FAILURE);
				}

				curl_off_t conntet_length_difference = http_content_length - last_request_length;

				free(http_header_last_modified_value);
				http_header_last_modified_value = get_last_modified_value(&messeges_id_header);
				printf("%s \n", http_header_last_modified_value);

				char * msg_id = (char *) malloc(sizeof(char)*(ID_LENGTH + 1)); // void * malloc (size_t size)
				if (msg_id == NULL) {
					fprintf(stderr, "Out of memory ! \n");
					exit(EXIT_FAILURE);
				}

				// char * strtok (char *restrict newstring, const char *restrict delimiters)
				msg_id = strtok(messeges_id.str + last_request_length,"\n");

				// If the end of the string newstring is reached,
				// or if the remainder of string consists only of delimiter characters,
				// strtok returns a null pointer. (Source : Doc via Eclipse)
				while(msg_id != NULL){
					printf("Id of messege %d : %s \n", ++count, msg_id);

					int url_length = (strlen(HOST_URL) + strlen(msg_id) + 1); // +1 for '\0'
					char* url = (char *) malloc(sizeof(char) * url_length);
					if (url == NULL) {
						fprintf(stderr, "Out of memory ! \n");
						exit(EXIT_FAILURE);
					}

					// int snprintf (char *s, size_t size, const char *template, ...)
					snprintf(url, url_length, "%s%s", HOST_URL, msg_id);

					printf("HTTP GET REQUEST TO : %s \n", url);
					curl_easy_setopt(curl, CURLOPT_URL, url);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &messege_body);
					curl_easy_setopt(curl, CURLOPT_HTTPHEADER, NULL); //  NULL : no custom headers.

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
					free(url);

					// Subsequent calls to get additional tokens from the same string are indicated by
					// passing a null pointer as the newstring argument.
					// Calling strtok with another non-null newstring argument reinitializes
					// the state information
					msg_id = strtok(NULL ,"\n");
				}

				last_request_length = http_content_length;
				free(msg_id);
			}
		}
		curl_easy_cleanup(curl);
	} else {
		// int fprintf (FILE *stream, const char *template, ...)
		fprintf(stderr, "Something went wrong \n");
		exit (EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
