#ifndef _REQUESTS_
#define _REQUESTS_

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(char *host, char *url, char *query_params,
							char *cookie, char *token_cookie);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(char *host, char *url, char* content_type, char *body_data,
							char* cookie, char *token_cookie);

// computes and returns a DELETE request string (cookies can be NULL if not needed)
char *compute_delete_request(char *host, char *url, char *cookie, char *token_cookie);

#endif