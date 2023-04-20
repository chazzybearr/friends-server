#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/friends.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>


/**
 * Returns the index of the character after "\r\n" in the buf
 * Returns -1 if it does not exist
 */
int find_network_newline(const char *buf, int n) {
    // Searching for \r
    for (int i = 0; i < n; i++) {

        if (buf[i] == '\r') {
            if (i < n-1 && buf[i+1] == '\n') {
                return i + 2;
            }
        }
    }
    return -1;
}

/**
 * Writes a formatted error message to the client socket
 */
void error(char *msg, int client_socket) {
    int size = strlen("Error: ") + strlen(msg) + strlen("\r\n");
    char *out;

    MALLOC(out, sizeof(char) * size);

    sprintf(out, "Error: %s\r\n", msg);
    if (write(client_socket, out, size + 1) == -1) {
        perror("write");
        exit(1);
    }
    free(out);
}

/**
 * Reads and processes commands given the user list
 * Writes appropriate error messages to the client socket
 * Returns 0 on success, -1 on quit command
 */
int process_args(int cmd_argc, char **cmd_argv, User **user_list_ptr, int client_socket, char *name) {
    User *user_list = *user_list_ptr;

    if (cmd_argc <= 0) {
        if (write(client_socket, "\r\n", strlen("\r\n")) == -1) {
            perror("write");
            exit(1);
        }
        return 0;
    } else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
        return -1;
    } else if (strcmp(cmd_argv[0], "add_user") == 0 && cmd_argc == 2) {
        switch (create_user(cmd_argv[1], user_list_ptr)) {
            case 1:
                error("user by this name already exists", client_socket);
                break;
            case 2:
                error("username is too long", client_socket);
                break;
        }

    } else if (strcmp(cmd_argv[0], "list_users") == 0 && cmd_argc == 1) {
        char *buf = list_users(user_list);
        char out[BUFSIZE];
        sprintf(out, "%s", buf);
        if (write(client_socket, out,strlen(out)) == -1) {
            perror("write");
            exit(1);
        }
        free(buf);

    } else if (strcmp(cmd_argv[0], "make_friends") == 0 && cmd_argc == 2) {


        switch (make_friends(name, cmd_argv[1], user_list)) {
            case 1:
                error("users are already friends", client_socket);
                break;
            case 2:
                error("at least one user you entered has the max number of friends", client_socket);
                break;
            case 3:
                error("you must enter two different users", client_socket);
                break;
            case 4:
                error("at least one user you entered does not exist", client_socket);
                break;
        }
    } else if (strcmp(cmd_argv[0], "post") == 0 && cmd_argc >= 3) {
        // first determine how long a string we need
        int space_needed = 0;
        for (int i = 2; i < cmd_argc; i++) {
            space_needed += strlen(cmd_argv[i]) + 1;
        }

        // allocate the space
        char *contents;
        MALLOC(contents, space_needed);

        // copy in the bits to make a single string
        strcpy(contents, cmd_argv[2]);
        for (int i = 3; i < cmd_argc; i++) {
            strcat(contents, " ");
            strcat(contents, cmd_argv[i]);
        }

        User *author = find_user(name, user_list);
        User *target = find_user(cmd_argv[1], user_list);
        switch (make_post(author, target, contents)) {
            case 1:
                error("the users are not friends", client_socket);
                break;
            case 2:
                error("at least one user you entered does not exist", client_socket);
                break;
        }
    } else if (strcmp(cmd_argv[0], "profile") == 0 && cmd_argc == 2) {
        User *user = find_user(cmd_argv[1], user_list);
        char *buf = print_user(user);
        char out[BUFSIZE];
        if (strcmp(buf,"") == 0) {
            error("user not found", client_socket);
        } else {
            sprintf(out, "%s", buf);
            if (write(client_socket, out, strlen(out)) == -1) {
                perror("write");
                exit(1);
            }
            free(buf);
        }
    } else {
        error("Incorrect syntax", client_socket);
    }
    return 0;
}


/*
 * Tokenize the string stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 */
int tokenize(char *cmd, char **cmd_argv, int client_socket) {
    int cmd_argc = 0;
    char *next_token = strtok(cmd, DELIM);
    while (next_token != NULL) {
        if (cmd_argc >= INPUT_ARG_MAX_NUM - 1) {
            error("Too many arguments!", client_socket);
            cmd_argc = 0;
            break;
        }
        cmd_argv[cmd_argc] = next_token;
        cmd_argc++;
        next_token = strtok(NULL, DELIM);
    }

    return cmd_argc;
}

/**
 * Accepts a connection on the listen_soc
 * Returns the newly connected client socket
 */
int accept_client(int listen_soc) {
    // Accept one client
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    unsigned int client_len = sizeof(struct sockaddr_in);

    int client_socket = accept(listen_soc, (struct sockaddr *) &client_addr, &client_len);
    if (client_socket < 0) {
        perror("accept");
        exit(1);
    }
    return client_socket;
}

/**
 * Creates a new client instance and adds it to the linked structure
 */
void add_client(int client_socket, char *name, Client **client_list_ptr) {


    Client *new_client;
    MALLOC(new_client, sizeof(Client));

    new_client->client_socket = client_socket;
    strncpy(new_client->name, name, BUFSIZE);
    new_client->next = NULL;
    new_client->commands = 0;


    if (*client_list_ptr == NULL) {
        *client_list_ptr = new_client;
        return;
    }

    Client *client_list = *client_list_ptr;

    while (client_list->next != NULL) {
        client_list = client_list->next;
    }
    client_list->next = new_client;
    return;
}

/**
 * Removes a client from the linked list of clients
 * Precondition: Client is in the client list
 */
void remove_client(int client_socket, Client **client_list_ptr) {

    Client *client_list = *client_list_ptr;

    if (client_list->client_socket == client_socket) {
        if (client_list->next == NULL) {
            free(client_list);
            *client_list_ptr = NULL;
            return;
        }
        *client_list_ptr = client_list->next;
        free(client_list);
        return;
    }

    while (client_list->next != NULL) {
        if (client_list->next->client_socket == client_socket) {
            Client *removed_client = client_list->next;
            client_list->next = client_list->next->next;
            free(removed_client);
            return;
        }
    }
    return;
}

/**
 * Returns the client instance from the linked list of clients
 * Returns NULL if it does not exist
 */
Client *find_client(int client_socket, Client **client_list_ptr) {

    if (client_list_ptr == NULL) {
        return NULL;
    }

    Client *curr = *client_list_ptr;
    while (curr != NULL) {
        if (curr->client_socket == client_socket) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

/**
 * Closes the client socket and removes the client from the client list
 */
void close_client(int client_socket, Client **client_list) {
    if (shutdown(client_socket, SHUT_RDWR) == -1) {
        perror("shutdown");
        exit(1);
    }
    if (close(client_socket) == -1) {
        perror("close");
        exit(1);
    }
    remove_client(client_socket, client_list);
    return;
}

/**
 * Reads a line from the client socket and returns the line
 * Close_sig set to 1 if the client closed their connection
 *
 * Precondition: client_socket must not be blocked
 */
char *read_command(int client_socket, Client **client_list_ptr, int *close_sig) {

    Client *client = find_client(client_socket, client_list_ptr);
    strcpy(client->buffer, "\0");
    // Receive messages
    char *next_command;
    int inbuf = 0;           // How many bytes currently in buffer?
    int room = sizeof(client->buffer);  // How many bytes remaining in buffer?
    char *after = client->buffer;       // Pointer to position after the data in buf
    int nbytes;
    while ((nbytes = read(client_socket, after, room)) > 0) {
        // Update inbuf
        inbuf += nbytes;
        int where;
        // Determine if a full line has been read from the client.
        while ((where = find_network_newline(client->buffer, inbuf)) > 0) {
            // Next command is the full line, not including the "\r\n",
            client->buffer[where - 2] = '\0';
            MALLOC(next_command, sizeof(char) * strlen(client->buffer) + 1);
            strcpy(next_command, client->buffer);
            return next_command;

            // Update inbuf and remove the full line from the buffer
            inbuf = inbuf - where;
            memmove(client->buffer, &(client->buffer[where]), inbuf);
        }
        // Update after and room, in preparation for the next read.
        after = &(client->buffer[inbuf]);
        room = sizeof(client->buffer) - inbuf;
    }

    // Read failed
    if (nbytes == -1) {
        perror("read");
        exit(1);
    }
    // Client closed connection only when nbytes returns 0
    *close_sig = 1;
    return "";
}

/**
 * Registers the client in the linked structure of Users and Clients
 */
void register_client(Client *registering, Client **registering_client_list_ptr, Client **client_list_ptr, User **user_list_ptr) {

    // Getting socket of the unregistered client
    int client_socket = registering->client_socket;

    // Reading their username
    int close_sig = 0;
    char *command = read_command(client_socket, registering_client_list_ptr, &close_sig);
    if (close_sig) {
        close_client(client_socket, registering_client_list_ptr);
        return;
    }

    // Removing from registering list
    remove_client(client_socket, registering_client_list_ptr);

    // Adding to registered list
    add_client(client_socket, command, client_list_ptr);

    // Add to user list
    create_user(command, user_list_ptr);
    if (write(client_socket, "Welcome! You may enter commands.\r\n", sizeof("Welcome! You may enter commands.\r\n")) == -1) {
        perror("write");
        exit(1);
    }

    free(command);
}




