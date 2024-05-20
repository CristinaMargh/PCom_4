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
#define BOOKS_ACCESS "/api/v1/tema/library/books"
// Used to save the user after login command. 
char *in_system = NULL;
char *token = NULL;
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
    char *request = compute_post_request(HOST, "/api/v1/tema/auth/register", "application/json", &serialized_string, 1, NULL, 0, NULL);
    //printf("%s", request);
    send_to_server(sockfd, request);
    free(request);

    char *response = receive_from_server(sockfd);
    //printf("%s", response);
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
    char *request = compute_post_request(HOST, "/api/v1/tema/auth/login", "application/json", &serialized_string, 1, NULL, 0, NULL);
    //printf("%s", request);
    send_to_server(sockfd, request);
    free(request);

    char *receive = receive_from_server(sockfd);
    //printf("%s", receive);
    
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
            in_system = malloc(BUFLEN * sizeof(char));
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
    if (!in_system) {
        printf("ERROR - No user loged!\n");
        return;
    }
    char *compute = compute_get_request(HOST,"/api/v1/tema/library/books",NULL, &in_system,  1, token);
    send_to_server(sockfd, compute);
    free(compute);
    char *response = receive_from_server(sockfd);
    //printf("%s", response);
     if (strstr(response, "\"error\"")) {
        printf("ERROR - Can't get the books\n");
    } else {
        // Assuming the books data starts with '['
        char *jsonData = strstr(response, "[");
        if(!jsonData)
            return;
        JSON_Value *root_value = json_parse_string(jsonData);
        if (json_value_get_type(root_value) == JSONArray) {
            JSON_Array *books = json_value_get_array(root_value);  
            int number_of_books = (int)json_array_get_count(books);  
            if (number_of_books == 0) {
                printf("No books in library!\n");
                return;
            }
            for (int i = 0; i < number_of_books; i++) {
                JSON_Object *book = json_array_get_object(books, i);
                printf("id:%d ", (int)json_object_get_number(book, "id"));
                printf("title:%s ", json_object_get_string(book, "title"));
                printf("\n");
            }
        } else {
            printf("Unexpected data format.\n");
        }
        json_value_free(root_value);   
    }
    free(response);
    close(sockfd);
}

void enter_library(int sockfd) {
    if(!in_system) {
        printf("No user!\n");
        return;
    }
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Failed to connect to the server.\n");
        exit(-1);
    } 
    char *compute = compute_get_request(HOST,"/api/v1/tema/library/access",NULL, &in_system,  1, NULL);
    //printf("%s", compute);
    send_to_server(sockfd, compute);
    free(compute);

    char *resp = receive_from_server(sockfd);
    //printf("%s", resp);
    if (resp == NULL) {
        printf("No response from server.\n");
    } else {
        if (strstr(resp, "error")) {
            printf("ERROR -No access to library!\n");
        } else {
            printf("Entered library.\n");       
            const char* tokenKey = "\"token\":\"";
            char* start = strstr(resp, tokenKey);
            if (start == NULL) {
                printf("Token not found.\n");
                return;
            }
            start += strlen(tokenKey);
            char* end = strchr(start, '\"');
            if (end == NULL) {
                printf("Invalid token format.\n");
                return;
            }
            int length = end - start;
            char* result = malloc(length + 10);
            DIE(!result, "malloc failed!");
            strncpy(result, start, length);
            result[length] = '\0';
            strcpy(result, "Bearer ");
            strncpy(result + 7, start, length);
            result[length + 7] = '\0'; 

            if (token != NULL) {
                free(token);
            }
            token = result;
            //printf("Token extracted: %s\n", token);   
            }
    }
    free(resp);
    close(sockfd);
}
int is_number(const char *str) {
    if (str == NULL) {
        return 0; 
    }

    char *endptr;
    errno = 0; 
    long val = strtol(str, &endptr, 10); // Conversia stringului în număr

    if (errno != 0 || endptr == str || *endptr != '\0' || str[0] == '\0') {
        return 0; 
    }
    return 1; 
}

void add_book(int sockfd) {
    char title[LINELEN], author[LINELEN], genre[LINELEN], publisher[LINELEN], page_count[LINELEN];
    // Open a socket connection
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Failed to connect to the server.\n");
        exit(-1);
    }
    if (!in_system) {
        printf("ERROR -No user!\n");
        return;
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
    if(!is_number(page_count)){
        printf("not number\n");
        return;
    }
    
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
    char *request = compute_post_request(HOST, BOOKS_ACCESS, "application/json", &serialized_string, 1, &in_system, 0, token);
    send_to_server(sockfd, request);
    free(request);

    char *receive = receive_from_server(sockfd);
    //printf("%s", receive);
    if (receive == NULL) {
        printf("No response from server.\n");
    } else {
        if (strstr(receive, "error") != NULL) {
            printf("ERROR - not entered the library!\n");
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

void get_book(int sockfd) {
    char id[LINELEN];
    printf("id=");
    fgets(id, LINELEN, stdin);
    trim_newline(id);

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf("Failed to connect to the server.\n");
            exit(-1);
        }
        if(in_system == NULL) {
            printf("ERROR - no one is loged!\n");
            exit(-1);
            }
        char new[LINELEN];
        strcpy(new, BOOKS_ACCESS);
        strcat(new, "/");
        strcat(new, id); 
        char *compute = compute_get_request(HOST, new, NULL, &in_system, 1, token);
        send_to_server(sockfd, compute);
        free(compute);

        char *resp = receive_from_server(sockfd);
       // printf("%s", resp);
        if (resp == NULL) {
            printf("No response from server.\n");
        } else {
            if (strstr(resp, "error")) {
                printf("ERROR -Can t get that book\n");
            } else {
                printf("SUCCESS - book with the given id identified!\n");
                // Separate headers and JSON content
    char *headerEnd = strstr(resp, "\r\n\r\n");
    if (headerEnd) {
        headerEnd += 4; // Move past the "\r\n\r\n"
        // Now headerEnd points to the beginning of the JSON body
        JSON_Value *root_value = json_parse_string(headerEnd);
        if (root_value != NULL) {
            JSON_Object *book = json_value_get_object(root_value);
            if (book != NULL) {
                printf("id: %d\n", (int)json_object_get_number(book, "id"));
                printf("title: %s\n", json_object_get_string(book, "title"));
                printf("author: %s\n", json_object_get_string(book, "author"));
                printf("publisher: %s\n", json_object_get_string(book, "publisher"));
                printf("genre: %s\n", json_object_get_string(book, "genre"));
                printf("page_count: %d\n", (int)json_object_get_number(book, "page_count"));
            } else {
                printf("No valid book data found.\n");
            }
            json_value_free(root_value);
        } else {
            printf("Failed to parse JSON data.\n");
        }
    } else {
        printf("No JSON data found in response.\n");
    }
            }
        }
        free(resp);
        close(sockfd);
}

void delete_book(int sockfd) {
    char id[LINELEN];
    printf("id=");
    fgets(id, LINELEN, stdin);
    trim_newline(id);
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf("Failed to connect to the server.\n");
            exit(-1);
        }
        if(in_system == NULL) {
            printf("ERROR - no one is loged!\n");
            exit(-1);
            }
        char new[LINELEN];
        strcpy(new, BOOKS_ACCESS);
        strcat(new, "/");
        strcat(new, id); 
        // need to delete the request
        
        // char *compute = compute_get_request(HOST,new, NULL, &in_system, 1, NULL);
        char *compute = compute_delete_request(HOST,new, NULL, &in_system, 1, token);

        send_to_server(sockfd, compute);
        free(compute);

        char *resp = receive_from_server(sockfd);
        if (resp == NULL) {
            printf("No response from server.\n");
        } else {
            if (strstr(resp, "error")) {
                printf("ERROR -Can t delete that book\n");
            } else {
                printf("SUCCESS - book deleted!\n");
            }
        }
        free(resp);
        close(sockfd);
}

void logout(int sockfd) {
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf("Failed to connect to the server.\n");
            exit(-1);
        }
        if (in_system == NULL) {
            printf("ERROR - no one is loged!\n");
            exit(-1);
            }
        char *compute = compute_get_request(HOST,"/api/v1/tema/auth/logout", NULL, &in_system, 1, NULL);
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
            printf("Error reading command input!\n");
            continue;
        }

        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }

        if (strcmp(command, "register") == 0) {
            register_comm(sockfd);
        } else if (strcmp(command, "enter_library") == 0) {
            enter_library(sockfd);
        } else if (strcmp(command, "login") == 0) {
            login(sockfd);
        } else if (strcmp(command, "get_books") == 0) {
            get_books(sockfd);
        } else if (strcmp(command, "get_book") == 0) {
            get_book(sockfd);
        } else if (strcmp(command, "add_book") == 0) {
            add_book(sockfd);
        } else if (strcmp(command, "delete_book") == 0) {
            delete_book(sockfd);
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
