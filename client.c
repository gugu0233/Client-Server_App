#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>

#define LEN 1024

// checks if the string is empty or contains spaces
int string_check(char *string, int space)
{
    // if string contains spaces
    if (space == 0 && strstr(string, " ") != NULL)
    {
        printf("ERROR: Credentialele nu pot contine spatii!\n");
        return -1;
    }

    // if string is empty
    if (strlen(string) == 0 || string[0] == '\n' || string[0] == ' ')
    {
        printf("ERROR: Campurile nu pot fi goale!\n");
        return -1;
    }

    return 0;
}

// returns a json string with username and password
char *getUserCredentials()
{
    char username[LEN] = {0};
    char password[LEN] = {0};

    // get username and password from stdin & remove newline
    printf("username=");
    fgets(username, LEN, stdin);
    printf("password=");
    fgets(password, LEN, stdin);
    username[strlen(username) - 1] = '\0';
    password[strlen(password) - 1] = '\0';

    // check if username and password are valid
    if (string_check(password, 0) == -1 || string_check(username, 0) == -1)
        return NULL;

    // create json object
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);

    // set username and password
    json_object_set_string(obj, "username", username);
    json_object_set_string(obj, "password", password);

    return json_serialize_to_string(val);
}

// extracts book id from url
int extract_id(const char *url)
{
    const char *last_slash = strrchr(url, '/');

    if (last_slash != NULL && *(last_slash + 1) != '\0')
        return atoi(last_slash + 1);

    return -1;
}

// returns url with book_id
char *get_url()
{
    // get book_id
    char id[1024] = {0};
    printf("id=");
    fgets(id, 1024, stdin);
    id[strlen(id) - 1] = '\0';

    // check if id contains only digits
    for (int i = 0; i < strlen(id); i++)
        if (!isdigit(id[i]))
        {
            printf("ERROR: Tip de date incorect pentru id-ul cartii!\n");
            return NULL;
        }

    // append id to url
    char url[LEN] = {0};
    strcpy(url, "/api/v1/tema/library/books/");
    strcat(url, id);

    return strdup(url);
}

// returns a json string with book info
char *get_book()
{
    char title[1024];
    char author[1024];
    char genre[1024];
    char publisher[1024];
    char page_count[1024];

    // get book info from stdin & remove newline
    printf("title=");
    fgets(title, LEN, stdin);
    title[strlen(title) - 1] = '\0';

    printf("author=");
    fgets(author, LEN, stdin);
    author[strlen(author) - 1] = '\0';

    printf("genre=");
    fgets(genre, LEN, stdin);
    genre[strlen(genre) - 1] = '\0';

    printf("publisher=");
    fgets(publisher, LEN, stdin);
    publisher[strlen(publisher) - 1] = '\0';

    printf("page_count=");
    fgets(page_count, LEN, stdin);
    page_count[strlen(page_count) - 1] = '\0';

    // check if book info is valid
    if (string_check(title, 1) == -1 || string_check(author, 1) == -1 ||
        string_check(genre, 1) == -1 || string_check(publisher, 1) == -1)
        return NULL;

    // check if page_count contains only digits
    for (int i = 0; i < strlen(page_count); i++)
    {
        if (!isdigit(page_count[i]))
        {
            printf("ERROR: Tip de date incorect pentru numarul de pagini!\n");
            return NULL;
        }
    }

    // create json object
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);

    // set book info
    json_object_set_string(obj, "title", title);
    json_object_set_string(obj, "author", author);
    json_object_set_string(obj, "genre", genre);
    json_object_set_string(obj, "publisher", publisher);
    json_object_set_string(obj, "page_count", page_count);

    return json_serialize_to_string(val);
}

int check_flags(int login_flag, int library_access, int sock) {
    // check if user is logged in
    if (login_flag == 0) {
        printf("ERROR: Utilizatorul nu este logat!\n");
        close_connection(sock);
        return 0;
    }

    // check if user already has access to the library
    if (library_access == 1) {
        printf("Utilizatorul are deja acces la biblioteca!\n");
        close_connection(sock);
        return 0;
    }

    return 1;
}


int main(int argc, char *argv[])
{
    int login_flag = 0;
    int library_access = 0;
    char cookie_flag[LEN] = {0};
    char library_token[LEN] = {0};

    char *response, *msg, *url, *info, *book;

    char ipaddr[16] = "34.246.184.49";

    while (1)
    {

        // get command from stdin
        char command[LEN] = {0};
        fgets(command, LEN, stdin);

        // open connection to server
        int sock = open_connection(ipaddr, 8080, AF_INET, SOCK_STREAM, 0);

        // exit command -> close connection and exit
        if (!strcmp(command, "exit\n"))
        {
            close_connection(sock);
            break;
        }

        else if (strcmp(command, "register\n") == 0) // register
        {
            // create json object with username and password
            info = getUserCredentials();
            if (info == NULL)
            {
                close_connection(sock);
                continue;
            }

            // create post request
            msg = compute_post_request(ipaddr, "/api/v1/tema/auth/register", "application/json", info, NULL, NULL);

            // send post request to server and receive response
            send_to_server(sock, msg);
            response = receive_from_server(sock);

            // check if username is already used, if not -> print success msg
            if (strstr(response, "HTTP/1.1 201") != NULL)
                printf("SUCCESS: Utilizator înregistrat cu succes!\n");
            else if (strstr(response, "{\"error\":\"The username") != NULL)
                printf("ERROR: Username-ul este folosit deja!\n");
        }

        else if (strcmp(command, "login\n") == 0) // login
        {
            // check if user is already logged in
            if (login_flag == 1)
            {
                printf("Utilizatorul este deja logat!\n");
                close_connection(sock);
                continue;
            }

            // get json object with username and password
            info = getUserCredentials();
            if (info == NULL)
            {
                close_connection(sock);
                continue;
            }

            // create post request
            msg = compute_post_request(ipaddr, "/api/v1/tema/auth/login", "application/json", info, NULL, NULL);

            // send post request to server and receive response
            send_to_server(sock, msg);
            response = receive_from_server(sock);

            // login successful -> cookie session
            if (strstr(response, "Set-Cookie: ") != NULL)
            {
                // extract cookie from response
                char *cookie = strtok(strstr(response, "Set-Cookie:"), " ");
                cookie = strtok(NULL, ";");
                strcpy(cookie_flag, cookie);

                // print success msg
                if (cookie_flag != NULL)
                {
                    login_flag = 1;
                    printf("SUCCESS: Utilizatorul a fost logat cu succes!\n");
                    printf("Bun venit!\n");
                }
            }
            // username not found or wrong password
            else if (strstr(response, "HTTP/1.1 400") != NULL)
                printf("ERROR: Eroare de logare! Credentiale incorecte!\n");
        }

        else if (strcmp(command, "enter_library\n") == 0) // enter_library
        {
            // check if user is logged in or already has access to library
            if (!check_flags(login_flag, library_access, sock))
            continue;

            // create get request
            msg = compute_get_request(ipaddr, "/api/v1/tema/library/access", NULL, cookie_flag, NULL);

            // send get request to server and receive response
            send_to_server(sock, msg);
            response = receive_from_server(sock);

            if (strstr(response, "HTTP/1.1 200") != NULL)
            {
                // get token from response
                char *token = strtok(strstr(response, "token"), ":");
                token = strtok(NULL, "\"");
                strcpy(library_token, token);

                // set library access to 1 and print success msg
                if (library_token != NULL)
                {
                    library_access = 1;
                    printf("SUCCESS: Utilizatorul are acces la biblioteca!\n");
                }
            }
        }

        else if (strcmp(command, "get_books\n") == 0) // get_books
        {
            // check if user has access to library
            if (library_access == 0)
            {
                printf("ERROR: Utilizatorul nu are acces la biblioteca!\n");
                close_connection(sock);
                continue;
            }

            // create get request
            msg = compute_get_request(ipaddr, "/api/v1/tema/library/books", NULL, cookie_flag, library_token);

            // send get request to server and receive response
            send_to_server(sock, msg);
            response = receive_from_server(sock);

            // print books
            char *books = strstr(response, "\n[{\"id\":");
            if (books == NULL)
                printf("ERROR: Nu exista carti in biblioteca!\n");
            else
            {
                JSON_Value *root_value;
                JSON_Array *books_array;
                JSON_Object *book_object;
                root_value = json_parse_string(books);
                books_array = json_value_get_array(root_value);

                printf("[\n");

                for (int i = 0; i < json_array_get_count(books_array); i++)
                {
                    book_object = json_array_get_object(books_array, i);

                    printf("  {\n");
                    printf("    \"id\": %d,\n", (int)json_object_get_number(book_object, "id"));
                    printf("    \"title\": \"%s\"\n", json_object_get_string(book_object, "title"));

                    if (i < json_array_get_count(books_array) - 1)
                        printf("  },\n");
                    else
                        printf("  }\n");
                }

                printf("]\n");
            }
        }
        else if (strcmp(command, "get_book\n") == 0) // get_book
        {
            // check if user has access to library
            if (library_access == 0)
            {
                printf("ERROR: Utilizatorul nu are acces la biblioteca!\n");
                close_connection(sock);
                continue;
            }

            // get url
            url = get_url();
            if (url == NULL)
            {
                close_connection(sock);
                continue;
            }

            // create get request
            msg = compute_get_request(ipaddr, url, NULL, cookie_flag, library_token);

            // send get request to server and receive response
            send_to_server(sock, msg);
            response = receive_from_server(sock);

            int id = extract_id(url);

            // print book info
            if (strstr(response, "HTTP/1.1 404") != NULL)
                printf("ERROR: Nu a fost gasita nicio carte cu id %d!\n", id);
            else if (strstr(response, "HTTP/1.1 200") != NULL)
            {
                book = strstr(response, "{\"id\":");

                if (book == NULL)
                    printf("ERROR: Nu a fost gasita nicio carte cu id %d!\n", id);
                else
                {
                    JSON_Value *root_value;
                    JSON_Object *book_object;
                    root_value = json_parse_string(book);
                    book_object = json_value_get_object(root_value);

                    printf("{\n");
                    printf("  \"id\": %d,\n", (int)json_object_get_number(book_object, "id"));
                    printf("  \"title\": \"%s\",\n", json_object_get_string(book_object, "title"));
                    printf("  \"author\": \"%s\",\n", json_object_get_string(book_object, "author"));
                    printf("  \"publisher\": \"%s\",\n", json_object_get_string(book_object, "publisher"));
                    printf("  \"genre\": \"%s\",\n", json_object_get_string(book_object, "genre"));
                    printf("  \"page_count\": %d\n", (int)json_object_get_number(book_object, "page_count"));
                    printf("}\n");
                }
            }
        }

        else if (strcmp(command, "add_book\n") == 0) // add_book
        {
            // check if user has access to library
            if (library_access == 0)
            {
                printf("ERROR: Utilizatorul nu are acces la biblioteca!\n\n");
                close_connection(sock);
                continue;
            }

            // create json object with book info
            info = get_book();
            if (info == NULL)
            {
                close_connection(sock);
                continue;
            }

            // create post request
            msg = compute_post_request(ipaddr, "/api/v1/tema/library/books", "application/json", info, cookie_flag, library_token);

            // send post request to server and receive response
            send_to_server(sock, msg);
            response = receive_from_server(sock);

            // print success msg
            if (strstr(response, "HTTP/1.1 200") != NULL)
                printf("SUCCESS: Cartea a fost adaugata!\n");
            else if (strstr(response, "HTTP/1.1 404") != NULL)
                printf("ERROR\n");
        }

        else if (strcmp(command, "delete_book\n") == 0) // delete_book
        {
            // check if user has access to library
            if (library_access == 0)
            {
                printf("ERROR: Utilizatorul nu are acces la biblioteca!\n");
                close_connection(sock);
                continue;
            }

            // get url
            url = get_url();
            if (url == NULL)
            {
                close_connection(sock);
                continue;
            }

            // create delete request
            msg = compute_delete_request(ipaddr, url, cookie_flag, library_token);

            // send delete request to server and receive response
            send_to_server(sock, msg);
            response = receive_from_server(sock);

            // print success msg
            int id = extract_id(url);

            if (strstr(response, "HTTP/1.1 404") != NULL)
                printf("ERROR: Nu a fost gasita nicio cartea cu id-ul %d!\n", id);
            else if (strstr(response, "HTTP/1.1 200") != NULL)
                printf("SUCCESS: Cartea cu id %d a fost stearsa!\n", id);
        }

        else if (strcmp(command, "logout\n") == 0) // logout
        {
            // check if user is logged in
            if (login_flag == 0)
            {
                printf("ERROR: Utilizatorul nu este logat!\n");
                close_connection(sock);
                continue;
            }

            // print success msg
            printf("SUCCESS: Utilizatorul s-a delogat cu succes!\n");

            // reset flags
            library_access = 0;
            login_flag = 0;

            // reset cookie and token
            library_token[0] = '\0';
            cookie_flag[0] = '\0';
        }

        else
            printf("ERROR: Comanda invalida!\n");

        printf("\n");
        close_connection(sock);
    }

    return 0;
}
