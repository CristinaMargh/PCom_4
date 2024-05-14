#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

#define HOST "34.246.184.49"
#define PORT 8080
#define ACCESS "/api/v1/tema/auth/%s"
// Used to save the user after login command. 
char *in_system = NULL;
// Delete newline from the end of a string
void trim_newline(char *string) {
    int len = strlen(string);
    if (len > 0 && string[len - 1] == '\n') {
        string[len - 1] = '\0';
    }
}
// Register commnad
void register_comm(int sockfd) {
    char username[LINELEN], password[LINELEN];
    // Open a socket connection
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Failed to connect to the server.\n");
        exit(-1);
    }
    // Username
    printf("username=");
    fgets(username, LINELEN, stdin);
    trim_newline(username);
    // Password
    printf("password=");
    fgets(password, LINELEN, stdin);
    trim_newline(password);
    // Create JSON object
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    char *serialized_string = json_serialize_to_string(root_value);
    // message
    char *request = compute_post_request(HOST, "/api/v1/tema/auth/register", "application/json", &serialized_string, 1, NULL, 0);
    send_to_server(sockfd, request);
    free(request);

    char *response = receive_from_server(sockfd);
    if (response == NULL) {
        printf("No response from server.\n");
    } else {
        // Bad request
        if (strstr(response, "Bad Request") != NULL) {
            printf("ERROR -repeated username!\n");
        } else { 
            printf("SUCCESS -registration done!\n");
        }
        free(response);
    }

    json_free_serialized_string(serialized_string);
    json_value_free(root_value);

    // Close the socket connection
    close(sockfd);
}
// Login command
void login(int sockfd) {
    char username[LINELEN], password[LINELEN];
    // Open a socket connection
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Failed to connect to the server.\n");
        exit(-1);
    }
    // Username
    printf("username=");
    fgets(username, LINELEN, stdin);
    trim_newline(username);

    // Password
    printf("password=");
    fgets(password, LINELEN, stdin);
    trim_newline(password);

    // Create JSON object
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    char *serialized_string = json_serialize_to_string(root_value);
    // message
    char *request = compute_post_request(HOST, "/api/v1/tema/auth/login", "application/json", &serialized_string, 1, NULL, 0);
    send_to_server(sockfd, request);
    free(request);

    char *receive = receive_from_server(sockfd);
    if (receive == NULL) {
        printf("No response from server.\n");
    } else {
        if (strstr(receive, "Bad Request") != NULL) {
            printf("ERROR\n");
        } else { 
            printf("SUCCESS - user loged in!\n");
            // We take the loged user
            const char* startKeyword = "connect.sid";
            char* start = strstr(receive, startKeyword);
            char* end = strchr(start, ';');
            int length = end - start;
            char* result = malloc(length + 1);
            strncpy(result, start, length);
            result[length] = '\0';

            if (in_system!= NULL) {
                free(in_system); 
            }
            in_system = result;
        }
        free(receive);
    }

    json_free_serialized_string(serialized_string);
    json_value_free(root_value);

    // Close the socket connection
    close(sockfd);
}

void get_books(int sockfd) {
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Failed to connect to the server.\n");
        exit(-1);
    }
    char *compute = compute_get_request(HOST,"/api/v1/tema/library/books",NULL, &in_system,  1);
    send_to_server(sockfd, compute);
    free(compute);
    char *resp = receive_from_server(sockfd);

    // Show the books in JSON format
    char *string = strstr(resp, "[");
    JSON_Value *value = json_parse_string(string);
    JSON_Array *all_books = json_value_get_array(value);
    json_value_free(value);
    close(sockfd);
}

void enter_library(int sockfd) {
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Failed to connect to the server.\n");
        exit(-1);
    }
    char *compute = compute_get_request(HOST,"/api/v1/tema/library/access",NULL, &in_system,  1);
    send_to_server(sockfd, compute);
    free(compute);

    char *resp = receive_from_server(sockfd);
    if (resp == NULL) {
        printf("No response from server.\n");
    } else {
        if (strstr(resp, "error")) {
            printf("ERROR -No access to library!\n");
        } else {
            printf("Entered library.\n");
        }
    }
    free(resp);
    close(sockfd);
}
void add_book(int sockfd) {
    char title[LINELEN], author[LINELEN], genre[LINELEN], publisher[LINELEN], page_count[LINELEN];
    // Open a socket connection
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Failed to connect to the server.\n");
        exit(-1);
    }
    printf("title=");
    fgets(title, LINELEN, stdin);
    trim_newline(title);

    printf("author=");
    fgets(author, LINELEN, stdin);
    trim_newline(author);

    printf("genre=");
    fgets(genre, LINELEN, stdin);
    trim_newline(genre);
    
    printf("publisher=");
    fgets(publisher, LINELEN, stdin);
    trim_newline(publisher);

    printf("page_count=");
    fgets(page_count, LINELEN, stdin);
    trim_newline(page_count);
    
    // Create JSON object
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_string(root_object, "publisher", publisher);
    json_object_set_string(root_object, "page_count", page_count);
    char *serialized_string = json_serialize_to_string(root_value);
    // message
    char *request = compute_post_request(HOST, "/api/v1/tema/library/books", "application/json", &serialized_string, 1, NULL, 0);
    send_to_server(sockfd, request);
    free(request);

    char *receive = receive_from_server(sockfd);
    if (receive == NULL) {
        printf("No response from server.\n");
    } else {
        if (strstr(receive, "Bad Request") != NULL) {
            printf("ERROR - book is not added!\n");
        } else { 
            printf("SUCCESS - added book!\n");
        }
        free(receive);
    }

    json_free_serialized_string(serialized_string);
    json_value_free(root_value);

    // Close the socket connection
    close(sockfd);
}
void logout(int sockfd) {
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf("Failed to connect to the server.\n");
            exit(-1);
        }
        if(in_system == NULL) {
            printf("ERROR - no one is loged!\n");
            exit(-1);
            }
        char *compute = compute_get_request(HOST,"/api/v1/tema/auth/logout", NULL, &in_system, 1);
        send_to_server(sockfd, compute);
        free(compute);

        char *resp = receive_from_server(sockfd);
        if (resp == NULL) {
            printf("No response from server.\n");
        } else {
            if (strstr(resp, "error")) {
                printf("We can t logout the user!\n");
            } else {
                printf("Logout done!\n");
            }
        }
        free(in_system);
        free(resp);
        close(sockfd);
}

int main() {
    char command[20];
    int sockfd;

    while (1) {
        if (!fgets(command, sizeof(command), stdin)) {
            printf("Error reading input or EOF encountered!\n");
            continue;
        }

        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }

        // Process commands
        if (strcmp(command, "register") == 0) {
            register_comm(sockfd);
        } else if (strcmp(command, "enter_library") == 0) {
            enter_library(sockfd);
        } else if (strcmp(command, "login") == 0) {
            login(sockfd);
        } else if (strcmp(command, "get_books") == 0) {
            get_books(sockfd);
        } else if (strcmp(command, "get_book") == 0) {

        } else if (strcmp(command, "add_book") == 0) {
            add_book(sockfd);
        } else if (strcmp(command, "delete_book") == 0) {

        } else if (strcmp(command, "logout") == 0) {
            logout(sockfd);
        }
        else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("Invalid command.\n");
        }
    }

    return 0;
}
