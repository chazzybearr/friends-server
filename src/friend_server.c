#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../include/friendme.h"

#ifndef PORT
    #define PORT 56523
#endif

// Helper functions defined in friendme.h
int main() {

    // Configuring the socket
    int listen_soc = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_soc == -1) {
        perror("socket");
        exit(1);
    }

    // Configuring socket to wait for connections
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(addr.sin_zero), 0, 8);

    if (bind(listen_soc, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1) {
        perror("bind");
        close(listen_soc);
        exit(1);
    }


    // Listening for connections
    if (listen(listen_soc, 5) < 0) {
        perror("listen");
        exit(1);
    }

    User *user_list = NULL;
    Client *client_list = NULL;
    Client *registering_client_list = NULL;



    // Set of fd's of clients to interact with
    fd_set client_fds;
    FD_ZERO(&client_fds);
    FD_SET(listen_soc, &client_fds);
    int maxfd = listen_soc + 1;

    // Main loop
    while (1) {

        // Setting all the fd's
        FD_ZERO(&client_fds);
        FD_SET(listen_soc, &client_fds);

        Client *cl = client_list;
        while (cl != NULL) {
            FD_SET(cl->client_socket, &client_fds);
            cl = cl->next;
        }


        cl = registering_client_list;
        while (cl != NULL) {
            FD_SET(cl->client_socket, &client_fds);
            cl = cl->next;
        }

        // Use select to check which fd is ready
        if (select(maxfd, &client_fds, NULL, NULL, NULL) != 1) {
            perror("select");
            exit(1);
        }

        // If the unblocked fd is the listen socket
        if (FD_ISSET(listen_soc, &client_fds)) {

            int client_socket = accept_client(listen_soc);
            // Update maxfd
            if (maxfd - 1 < client_socket) {
                maxfd = client_socket + 1;
            }

            // Add client to the list of registering clients
            add_client(client_socket, "", &registering_client_list);

            // Prompt client for username
            if (write(client_socket, "What is your username?\r\n", sizeof("What is your username?\r\n")) == -1) {
                perror("write");
                exit(1);
            }
            continue;
        }


        // Otherwise, the unblocked fd is a client

        // Checking if the client has registered
        Client *registering = registering_client_list;
        while (registering != NULL) {
            if (FD_ISSET(registering->client_socket, &client_fds)) {
                break;
            }
            registering = registering->next;
        }

        // If the client has not registered, register the client
        if (registering != NULL) {
            register_client(registering, &registering_client_list, &client_list, &user_list);
            continue;
        }

        // The client has already registered
        Client *curr = client_list;
        while (curr != NULL) {
            if (FD_ISSET(curr->client_socket, &client_fds)) {
                break;
            }
            curr = curr->next;
        }
        int client_socket = curr->client_socket;
        char *name = curr->name;

        // Reading a command
        int close_sig = 0;
        char *command = read_command(client_socket, &client_list, &close_sig);
        if (close_sig) {
            close_client(client_socket, &client_list);
            continue;
        }

        // Processing command
        char *cmd_argv[INPUT_ARG_MAX_NUM];
        int cmd_argc = tokenize(command, cmd_argv, client_socket);

        // Execute command
        if (process_args(cmd_argc, cmd_argv, &user_list, client_socket, name) == -1) {
            // Only here if quit command was typed in
            close_client(client_socket, &client_list);
            free(command);
            continue;
        }
        free(command);

    }

    return 0;
}
