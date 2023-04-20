#include "friends.h"

/**
 * Returns the index of the character after "\r\n" in the buf
 * Returns -1 if it does not exist
 */
int find_network_newline(const char *buf, int n);

/**
 * Reads and processes commands given the user list
 * Writes appropriate error messages to the client socket
 * Returns 0 on success, -1 on quit command
 */
int process_args(int cmd_argc, char **cmd_argv, User **user_list_ptr, int client_socket, char *name);

/**
 * Writes a formatted error message to the client socket
 */
void error(char *msg, int client_socket);

/*
 * Tokenize the string stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 */
int tokenize(char *cmd, char **cmd_argv, int client_socket);

/**
 * Accepts a connection on the listen_soc
 * Returns the newly connected client socket
 */
int accept_client(int listen_soc);

/**
 * Creates a new client instance and adds it to the linked structure
 */
void add_client(int client_socket, char *name, Client **client_list_ptr);

/**
 * Removes a client from the linked list of clients
 * Precondition: Client is in the client list
 */
void remove_client(int client_socket, Client **client_list_ptr);

/**
 * Returns the client instance from the linked list of clients
 * Returns NULL if it does not exist
 */
void find_client(int client_socket, Client **client_list_ptr);

/**
 * Closes the client socket and removes the client from the client list
 */
void close_client(int client_socket, Client **client_list_ptr);

/**
 * Reads a line from the client socket and returns the line
 * Close_sig set to 1 if the client closed their connection
 *
 * Precondition: client_socket must not be blocked
 */
char *read_command(int client_socket, Client **client_list_ptr, int *close_sig);

/**
 * Registers the client in the linked structure of Users and Clients
 */
void register_client(Client *registering, Client **registering_client_list_ptr, Client **client_list_ptr, User **user_list_ptr);