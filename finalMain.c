/* *********************************************************************************
 * Author: tyreej@msoe.edu
 * Last Edited: 12/11/2024
 * Project: Voting Machine (Lab 13)
 * Section: 121
 * Provides: A server terminal that multiple client terminals can connect to.
 * Collects votes from other terminals and also displays the result as votes
 * are collected. 
 * ********************************************************************************/
 



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8082 // Will probably have to change this a couple of times
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10


void *handle_client(void *socket_desc);
pthread_mutex_t lock;

int main() {
    int server_socket, client_socket, *new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    pthread_mutex_init(&lock, NULL);

    //Creating the socket
    if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    // Configuring server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    // Listening for incoming connectins
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server is running on port %d\n", PORT);

    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) >= 0) {
        printf("New client connected.\n");

        pthread_t client_thread;
        new_socket = malloc(sizeof(int));
        *new_socket = client_socket;

        if (pthread_create(&client_thread, NULL, handle_client, (void *)new_socket) < 0) {
            perror("Could not create new thread");
            free(new_socket);
        }
    }
    close(server_socket);
    pthread_mutex_destroy(&lock);
    return 0;
}


typedef struct {
    char candidate_name[50];
    int vote_count;
} Candidate;

// Candidate List
// candidate name, vote count (format)
Candidate candidates[] = {
    {"John Waltz", 0}, 
    {"Bob Turney", 0}, 
    {"Russell Meier", 0},
};

int candidate_count = sizeof(candidates) / sizeof(candidates[0]);



void *handle_client(void *socket_desc) {
    int client_socket = *(int *)socket_desc;
    free(socket_desc);

    char buffer[BUFFER_SIZE];
    int bytes_read;
    int has_voted = 0; // Flag to track if the "client" has voted. 

    //Receiving and processing client messages
    while ((bytes_read = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_read] = '\0';

        if (strncmp(buffer, "VOTE:", 5) == 0) { //5 is the character count
            if (has_voted) {
                send(client_socket, "You have already voted. Please view the results after the election", 66, 0);
                continue;
            }
            char *candidate_name = buffer + 5; // Getting the candidate name
            char *trimmed_candidate_name = strtok(candidate_name, "\n");
            int found = 0;

            pthread_mutex_lock(&lock);
            for (int i = 0; i < candidate_count; i++) {
                if(strcmp(candidates[i].candidate_name, trimmed_candidate_name) == 0) {
                    candidates[i].vote_count++;
                    found = 1;
                    has_voted = 1;
                    break;
                }
            }
            pthread_mutex_unlock(&lock);

            if (found) {
                send(client_socket, "Vote accepted", 13, 0);
                char results[BUFFER_SIZE] = "";
                for (int i = 0; i < candidate_count; i++) {
                    char line[100];
                    snprintf(line, sizeof(line), "%s: %d votes\n", candidates[i].candidate_name, candidates[i].vote_count);
                    if (strlen(results) + strlen(line) < BUFFER_SIZE) {
                        strcat(results, line);
                    }
                }
                printf("Results: \n%s\n", results);
            } else {
                send(client_socket, "Candidate Not Found", 20, 0);
            }
        } else if (strncmp(buffer, "RESULTS", 7) == 0) {
            //printf("Received command: %s\n", buffer);
            char results[BUFFER_SIZE] = "";
            pthread_mutex_lock(&lock);
            for (int i = 0; i < candidate_count; i++) {
                char line[100];
                snprintf(line, sizeof(line), "%s: %d votes\n", candidates[i].candidate_name, candidates[i].vote_count);
                if (strlen(results) + strlen(line) < BUFFER_SIZE) {
                    strcat(results, line);
                }
            }
            pthread_mutex_unlock(&lock);
            //printf("Sending results:\n%s\n", results);
            send(client_socket, results, strlen(results), 0);
        } else {
            send(client_socket, "Unknown command.\n", 18, 0);
        }
        memset(buffer, 0, sizeof(buffer));
    }
    close(client_socket);
    return NULL;
}


