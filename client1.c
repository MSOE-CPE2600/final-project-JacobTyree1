/* *********************************************************************************
 * Author: tyreej@msoe.edu
 * Last Edited: 12/11/2024
 * Project: Voting Machine (Lab 13)
 * Section: 121
 * Provides: A client terminal that connects to the server terminal through sockets. 
 * Allows the user to vote for the candidate of their choosing.
 * Security feature where users can only vote once per terminal. 
 * ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8082 // Will probably have to change this a couple of times
#define BUFFER_SIZE 1024

void show_menu();


int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE], input[BUFFER_SIZE];

    //Creating socket

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0 )) < 0) { //TCP sockets
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    printf("Connected to the server.\n");

    while (1) {
        show_menu();
        printf("Enter your choice: ");
        fgets(input, sizeof(input), stdin);

        if (strncmp(input, "1", 1) == 0) {
            printf("Enter candidate name: ");
            fgets(buffer, sizeof(buffer), stdin);

            buffer[strcspn(buffer, "\n")] = 0; // Removing the newline character
            
            if (strlen(buffer) == 0) {
                printf("Candidate name cannot be empty. Try again\n");
                continue;
            }
            
            char message[BUFFER_SIZE + 6];
            snprintf(message, sizeof(message), "VOTE:%s", buffer);

            if (send(client_socket, message, strlen(message), 0) < 0) {
                perror("Failed to send vote");
                continue;
            }
            int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
            if(bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf("%s\n", buffer);
            } else {
                printf("Failed to receive server response.\n");
            }
            memset(buffer, 0, sizeof(buffer));

        } else if (strncmp(input, "2", 1) == 0) {
            printf("Exiting...\n");
            break;
        } else {
            printf("Invalid choice.\n");
            continue;
        } 
        
        
        
        //else if (strncmp(input, "2", 1) == 0) {
            //send(client_socket, "RESULTS", 7, 0); //Sending the message to the other terminal
            //printf("Sent RESULTS command.\n");
            //int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
            //printf("Received from server:\n%s\n", buffer);
            //if (bytes_read > 0) {
              //  buffer[bytes_read] = '\0';
                //printf("\nVote Results:\n%s\n", buffer);
            //} else {
              //  printf("Failed to retrieve results.\n");
            //}
        

        
    }
    close(client_socket);
    return 0;
}

void show_menu() {
    printf("1. Vote for a candidate\n");
    printf("2. Exit\n");
}





