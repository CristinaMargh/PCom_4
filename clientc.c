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
// Function used to check if the page_number input from the add_book command is an actual number, not a string
int is_number(const char *str) {
    if (str == NULL) {
        return 0; 
    }
    char *endptr;
    errno = 0; 
    // Conversion
    strtol(str, &endptr, 10);
    if (errno != 0 || endptr == str || *endptr != '\0' || str[0] == '\0') {
        return 0; 
    }
    return 1; 
}
// Used to verify if the username or password has a space which is not permitted.
int there_is_space(const char *str) {
    if (str == NULL) {
        return 0; 
    }
    if (strchr(str, ' ') != NULL) {
        return 1;
    }
    // No space found
    return 0;
}
// Delete newline from the end of a string
void delete_newline(char *string) {
    int len = strlen(string);
    if (len > 0 && string[len - 1] == '\n') {
        string[len - 1] = '\0';
    }
}
// Register commnad function
void register_comm() {
    char username[LINELEN], password[LINELEN];
    // Open a socket connection
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Failed to connect to the server.\n");
        exit(-1);
    }
    // Username
    printf("username=");
    fgets(username, LINELEN, stdin);
    delete_newline(username);
    // Password
    printf("password=");
    fgets(password, LINELEN, stdin);
    delete_newline(password);
    // Checks 
    if (there_is_space(username) == 1) {
        printf("ERROR - there is a space in the username!\n");
        close(sockfd);
        return;
    }
    if (there_is_space(password) == 1) {
        printf("ERROR - there is a space in the password!\n");
        close(sockfd);
        return;
    }
    // Create JSON object
    JSON_Value *value = json_value_init_object();
    JSON_Object *object = json_value_get_object(value);
    json_object_set_string(object, "username", username);
    json_object_set_string(object, "password", password);
    char *serialized_string = json_serialize_to_string(value);
    // message
    char *request = compute_post_request(HOST, "/api/v1/tema/auth/register", "application/json",
         &serialized_string, 1, NULL, 0, NULL);
    send_to_server(sockfd, request);
    free(request);
    // Server's response check.
    char *response = receive_from_server(sockfd);
    if (response == NULL) {
        printf("ERROR -  No response from server.\n");
    } else {
        // Error
        if (strstr(response, "Bad Request") != NULL) {
            printf("ERROR -repeated username!\n");
        } else { 
            printf("SUCCESS -registration done!\n");
        }
        free(response);
    }
    // Free the resources.
    json_free_serialized_string(serialized_string);
    json_value_free(value);

    // Close the socket connection.
    close(sockfd);
}
// Login command function
void login() {
    char username[LINELEN], password[LINELEN];
    // Open a socket connection.
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Failed to connect to the server.\n");
        exit(-1);
    }
    // Username
    printf("username=");
    fgets(username, LINELEN, stdin);
    delete_newline(username);

    // Password
    printf("password=");
    fgets(password, LINELEN, stdin);
    delete_newline(password);

    if (there_is_space(username) == 1) {
        printf("ERROR - there is a space in the username!\n");
        close(sockfd);
        return;
    }
    if (there_is_space(password) == 1) {
        printf("ERROR - there is a space in the password!\n");
        close(sockfd);
        return;
    }

    // Create JSON object
    JSON_Value *value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    char *serialized_string = json_serialize_to_string(value);
    // message
    char *request = compute_post_request(HOST, "/api/v1/tema/auth/login", "application/json", &serialized_string, 1, NULL, 0, NULL);
    send_to_server(sockfd, request);
    free(request);

    char *receive = receive_from_server(sockfd);
    // Checks for the server's response.
    if (receive == NULL) {
        printf("No response from server.\n");
    } else {
        if (strstr(receive, "Bad Request") != NULL) {
            printf("ERROR - can't log the user!\n");
        } else { 
            printf("SUCCESS - user loged in!\n");
            // We take the loged user
            const char* start_word = "connect.sid";
            char* start = strstr(receive, start_word);
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
    // Free the used resources.
    json_free_serialized_string(serialized_string);
    json_value_free(value);

    // Close the socket connection
    close(sockfd);
}
// get_books command function
void get_books() {
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
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
    // Checks for the response.
     if (strstr(response, "error")) {
        printf("ERROR - Can't get the books\n");
    } else {
        // Books start with [
        char *jsonData = strstr(response, "[");
        if (!jsonData)
            return;

        JSON_Value *value = json_parse_string(jsonData);
        if (json_value_get_type(value) == JSONArray) {
            JSON_Array *books = json_value_get_array(value);  
            int number_of_books = (int)json_array_get_count(books);  
            if (number_of_books == 0) {
                printf("ERROR - No books in library!\n");
                return;
            }
            // We go through the books and display the required information
            for (int i = 0; i < number_of_books; i++) {
                JSON_Object *book = json_array_get_object(books, i);
                printf("id:%d ", (int)json_object_get_number(book, "id"));
                printf("title:%s ", json_object_get_string(book, "title"));
                printf("\n");
            }
        } else {
            printf("ERROR \n");
        }
        json_value_free(value);   
    }
    free(response);
    // Close the connection
    close(sockfd);
}
// enter_library command
void enter_library() {
    if(!in_system) {
        printf("No user!\n");
        return;
    }
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Failed to connect to the server.\n");
        exit(-1);
    } 
    char *compute = compute_get_request(HOST,"/api/v1/tema/library/access",NULL, &in_system,  1, NULL);
    //printf("%s", compute);
    send_to_server(sockfd, compute);
    free(compute);

    char *resp = receive_from_server(sockfd);
    if (resp == NULL) {
        printf("No response from server.\n");
    } else {
        if (strstr(resp, "error")) {
            printf("ERROR -No access to library!\n");
        } else {
            printf("SUCCESS - Entered library.\n");       
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
            }
    }
    free(resp);
    close(sockfd);
}
// Function for the add_book command.
void add_book() {
    char title[LINELEN], author[LINELEN], genre[LINELEN], publisher[LINELEN], page_count[LINELEN];
    // Open a socket connection
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Failed to connect to the server.\n");
        exit(-1);
    }
    printf("title=");
    fgets(title, LINELEN, stdin);
    delete_newline(title);

    printf("author=");
    fgets(author, LINELEN, stdin);
    delete_newline(author);

    printf("genre=");
    fgets(genre, LINELEN, stdin);
    delete_newline(genre);
    
    printf("publisher=");
    fgets(publisher, LINELEN, stdin);
    delete_newline(publisher);

    printf("page_count=");
    fgets(page_count, LINELEN, stdin);
    delete_newline(page_count);
    if (!is_number(page_count)){
        printf("ERROR - page_count is not number\n");
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
    char *cookie = malloc(BUFLEN * sizeof(char));
    if(in_system)
        strcpy(cookie, in_system);
    char *request = compute_post_request(HOST, BOOKS_ACCESS, "application/json", &serialized_string, 1, &cookie, 1, token);
    send_to_server(sockfd, request);
    free(request);

    char *receive = receive_from_server(sockfd);
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
// Function for the get_book command.
void get_book() {
    char id[LINELEN];
    printf("id=");
    fgets(id, LINELEN, stdin);
    delete_newline(id);

    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf("Failed to connect to the server.\n");
            exit(-1);
        }
        // Adding the id to form the exact url (char *url from the function definition)    
        char new[LINELEN];
        strcpy(new, BOOKS_ACCESS);
        strcat(new, "/");
        strcat(new, id); 

        char *compute = compute_get_request(HOST, new, NULL, &in_system, 1, token);
        send_to_server(sockfd, compute);
        free(compute);

         if (!in_system) {
            printf("ERROR - no one is loged!\n");
            exit(-1);
            }

        char *resp = receive_from_server(sockfd);
        // Checks for the response.
        if (resp == NULL) {
            printf("No response from server.\n");
        } else {
            if (strstr(resp, "error")) {
                printf("ERROR -Can t get that book\n");
            } else {
                printf("SUCCESS - book with the given id identified!\n");
    
                char *headerEnd = strstr(resp, "\r\n\r\n");
                 headerEnd += 4; // Move past the "\r\n\r\n"
                // Now headerEnd points to the beginning of the JSON body
                JSON_Value *value = json_parse_string(headerEnd);
                if (value != NULL) {
                    JSON_Object *book = json_value_get_object(value);
                    if (book != NULL) {
                        printf("id: %d\n", (int)json_object_get_number(book, "id"));
                        printf("title: %s\n", json_object_get_string(book, "title"));
                        printf("author: %s\n", json_object_get_string(book, "author"));
                        printf("publisher: %s\n", json_object_get_string(book, "publisher"));
                        printf("genre: %s\n", json_object_get_string(book, "genre"));
                        printf("page_count: %d\n", (int)json_object_get_number(book, "page_count"));
                    } else {
                        printf("ERROR - No valid book data found.\n");
                    }
                    json_value_free(value);
                    } else {
                        printf("Failed to parse JSON data.\n");
                    } 
                }
            }
        free(resp);
        // Close the connection.
        close(sockfd);
}
// Function for the delete_book command.
void delete_book() {
    char id[LINELEN];
    printf("id=");
    fgets(id, LINELEN, stdin);
    delete_newline(id);
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf("Failed to connect to the server.\n");
            exit(-1);
        }
        char new[LINELEN];
        strcpy(new, BOOKS_ACCESS);
        strcat(new, "/");
        strcat(new, id); 
        // need to delete the request
        char *compute = compute_delete_request(HOST,new, NULL, &in_system, 1, token);

        send_to_server(sockfd, compute);
        free(compute);

    if(in_system == NULL) {
            printf("ERROR - no one is loged!\n");
            exit(-1);
            }
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
// Function for the logout command.
void logout() {
    int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf("Failed to connect to the server.\n");
            exit(-1);
        }
        if (in_system == NULL) {
            printf("ERROR - no one is loged!\n");
            return;
            }
        char *compute = compute_get_request(HOST,"/api/v1/tema/auth/logout", NULL, &in_system, 1, NULL);
        send_to_server(sockfd, compute);
        free(compute);

        char *resp = receive_from_server(sockfd);
        if (resp == NULL) {
            printf("No response from server.\n");
        } else {
            if (strstr(resp, "error")) {
                printf("ERROR -  We can't logout the user!\n");
            } else {
                printf("SUCCESS - Logout done!\n");
            }
        }
        free(in_system);
        in_system = NULL;
        free(token);
        token = NULL;
        free(resp);
        close(sockfd);
}

int main() {
    char command[20];
   
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
            register_comm();
        } else if (strcmp(command, "enter_library") == 0) {
            enter_library();
        } else if (strcmp(command, "login") == 0) {
            login();
        } else if (strcmp(command, "get_books") == 0) {
            get_books();
        } else if (strcmp(command, "get_book") == 0) {
            get_book();
        } else if (strcmp(command, "add_book") == 0) {
            add_book();
        } else if (strcmp(command, "delete_book") == 0) {
            delete_book();
        } else if (strcmp(command, "logout") == 0) {
            logout();
        }
        else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("Invalid command.\n");
        }
    }

    return 0;
}
