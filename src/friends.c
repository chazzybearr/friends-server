#include "../include/friends.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Create a new user with the given name.  Insert it at the tail of the list
 * of users whose head is pointed to by *user_ptr_add.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if a user by this name already exists in this list.
 *   - 2 if the given name cannot fit in the 'name' array
 *       (don't forget about the null terminator).
 */
int create_user(const char *name, User **user_ptr_add) {
    if (strlen(name) >= MAX_NAME) {
        return 2;
    }

    User *new_user;
    MALLOC(new_user, sizeof(User));

    strncpy(new_user->name, name, MAX_NAME); // name has max length MAX_NAME - 1

    for (int i = 0; i < MAX_NAME; i++) {
        new_user->profile_pic[i] = '\0';
    }

    new_user->first_post = NULL;
    new_user->next = NULL;
    for (int i = 0; i < MAX_FRIENDS; i++) {
        new_user->friends[i] = NULL;
    }

    // Add user to list
    User *prev = NULL;
    User *curr = *user_ptr_add;
    while (curr != NULL && strcmp(curr->name, name) != 0) {
        prev = curr;
        curr = curr->next;
    }

    if (*user_ptr_add == NULL) {
        *user_ptr_add = new_user;
        return 0;
    } else if (curr != NULL) {
        free(new_user);
        return 1;
    } else {
        prev->next = new_user;
        return 0;
    }
}


/*
 * Return a pointer to the user with this name in
 * the list starting with head. Return NULL if no such user exists.
 */
User *find_user(const char *name, const User *head) {
    while (head != NULL && strcmp(name, head->name) != 0) {
        head = head->next;
    }

    return (User *)head;
}


/*
 * Print the usernames of all users in the list starting at curr.
 * Names should be printed to standard output, one per line.
 */
char *list_users(const User *curr) {

    int size = 11; // Number of characters in "User List\r\n"
    char *buffer;

    const User *temp = curr;

    while (temp != NULL) {
        size += strlen(temp->name) + 3; // Length of characters in username in addition to \t, \r\n
        temp = temp->next;
    }

    MALLOC(buffer, sizeof(char) * size + 1);

    strcpy(buffer, "User List\r\n");

    while (curr != NULL) {
        char name[strlen(curr->name) + 4];
        snprintf(name, strlen(curr->name) + 4, "\t%s\r\n", curr->name);
        strncat(buffer, name, strlen(name) + 1);
        curr = curr->next;
    }
    return buffer;
}


/*
 * Make two users friends with each other.  This is symmetric - a pointer to
 * each user must be stored in the 'friends' array of the other.
 *
 * New friends must be added in the first empty spot in the 'friends' array.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if the two users are already friends.
 *   - 2 if the users are not already friends, but at least one already has
 *     MAX_FRIENDS friends.
 *   - 3 if the same user is passed in twice.
 *   - 4 if at least one user does not exist.
 *
 * NOTE: If multiple errors apply, return the *largest* error code that applies.
 */
int make_friends(const char *name1, const char *name2, User *head) {
    User *user1 = find_user(name1, head);
    User *user2 = find_user(name2, head);

    if (user1 == NULL || user2 == NULL) {
        return 4;
    } else if (user1 == user2) { // Same user
        return 3;
    }

    int i, j;
    for (i = 0; i < MAX_FRIENDS; i++) {
        if (user1->friends[i] == NULL) { // Empty spot
            break;
        } else if (user1->friends[i] == user2) { // Already friends.
            return 1;
        }
    }

    for (j = 0; j < MAX_FRIENDS; j++) {
        if (user2->friends[j] == NULL) { // Empty spot
            break;
        }
    }

    if (i == MAX_FRIENDS || j == MAX_FRIENDS) { // Too many friends.
        return 2;
    }

    user1->friends[i] = user2;
    user2->friends[j] = user1;
    return 0;
}


/*
 *  Print a post.
 *  Use localtime to print the time and date.
 */
char *print_post(const Post *post) {

    int size = 0;
    char *buffer;

    if (post == NULL) {
        char *ret;
        MALLOC(ret, sizeof(char));
        strcpy(ret, "");
        return ret;
    }

    // Allocating the space for the buffer
    size += strlen("From: ") + strlen(post->author) + strlen("\r\n");
    size += strlen("Date: ") + strlen(asctime(localtime(post->date))) + strlen("\r\n");
    size += strlen(post->contents) + strlen("\r\n");

    MALLOC(buffer, sizeof(char) * size + 1); // Accounting for null terminator

    // Adding strings to buffer
    strcpy(buffer, "From: ");
    strcat(buffer, post->author);
    strcat(buffer, "\r\n");
    strcat(buffer, "Date: ");
    strcat(buffer, asctime(localtime(post->date)));
    strcat(buffer, "\r\n");
    strcat(buffer, post->contents);
    strcat(buffer, "\r\n");

    return buffer;
}


/*
 * Print a user profile.
 * Return:
 *   - 0 on success.
 *   - 1 if the user is NULL.
 */
char *print_user(const User *user) {

    size_t size = 0;
    char *buffer;

    if (user == NULL) {
        char *ret;
        MALLOC(ret, sizeof(char));
        strcpy(ret, "");
        return ret;
    }

    // Calculating size of memory required
    size += strlen("Name: ") + strlen(user->name) + strlen("\r\n\r\n") + strlen("------------------------------------------\r\n");
    size += strlen("Friends:\r\n");

    for (int i = 0; i < MAX_FRIENDS && user->friends[i] != NULL; i++) {
        size += strlen(user->friends[i]->name) + strlen("\r\n");
    }
    size += strlen("------------------------------------------\r\n");
    size += strlen("Posts:\r\n");

    const Post *temp = user->first_post;
    while (temp != NULL) {
        size += strlen(print_post(temp));
        temp = temp->next;
        if (temp != NULL) {
            size += strlen("\r\n===\r\n\r\n");
        }
    }
    size += strlen("------------------------------------------\r\n");

    // Allocate memory for the output string
    MALLOC(buffer, sizeof(char) * size + 1);

    // Adding strings to buffer
    strcpy(buffer, "Name: ");
    strcat(buffer, user->name);
    strcat(buffer, "\r\n\r\n");
    strcat(buffer, "------------------------------------------\r\n");


    strcat(buffer, "Friends:\r\n");

    for (int i = 0; i < MAX_FRIENDS && user->friends[i] != NULL; i++) {
        strcat(buffer, user->friends[i]->name);
        strcat(buffer, "\r\n");
    }
    strcat(buffer, "------------------------------------------\r\n");

    strcat(buffer, "Posts:\r\n");
    const Post *curr = user->first_post;
    while (curr != NULL) {
        strcat(buffer, print_post(curr));
        curr = curr->next;
        if (curr != NULL) {
            strcat(buffer, "\r\n===\r\n\r\n");
        }
    }
    strcat(buffer, "------------------------------------------\r\n");

    return buffer;
}


/*
 * Make a new post from 'author' to the 'target' user,
 * containing the given contents, IF the users are friends.
 *
 * Insert the new post at the *front* of the user's list of posts.
 *
 * Return:
 *   - 0 on success
 *   - 1 if users exist but are not friends
 *   - 2 if either User pointer is NULL
 */
int make_post(const User *author, User *target, char *contents) {
    if (target == NULL || author == NULL) {
        return 2;
    }

    int friends = 0;
    for (int i = 0; i < MAX_FRIENDS && target->friends[i] != NULL; i++) {
        if (strcmp(target->friends[i]->name, author->name) == 0) {
            friends = 1;
            break;
        }
    }

    if (friends == 0) {
        return 1;
    }

    // Create post
    Post *new_post;
    MALLOC(new_post, sizeof(Post));

    strncpy(new_post->author, author->name, MAX_NAME);
    new_post->contents = contents;

    MALLOC(new_post->date, sizeof(time_t));

    time(new_post->date);
    new_post->next = target->first_post;
    target->first_post = new_post;

    return 0;
}

