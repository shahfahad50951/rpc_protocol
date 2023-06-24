#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
using namespace std;

#define PORT 4000 
#define BUFFER_SIZE 1024
#define MESSAGE_SIZE 1024

void performRemoteProcedureCall(int clientSocket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Receive the RPC request from the client
    int bytesRead = read(clientSocket, buffer, BUFFER_SIZE);
    if (bytesRead < 0) {
        perror("Error reading from socket");
        close(clientSocket);
        exit(1);
    }

    // Process the RPC request
    // ... Add your custom RPC logic here ...
    string response;
    cout << "Enter the Message to Send to the User: ";
    cin >> response;

    // Send the RPC response back to the client
    int bytesWritten = write(clientSocket, response.c_str(), response.length());
    if (bytesWritten < 0) {
        perror("Error writing to socket");
        close(clientSocket);
        exit(1);
    }

    close(clientSocket);
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Prepare the server address structure
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // Bind the socket to the specified address and port
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Socket binding failed");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 3) < 0) {
        perror("Socket listening failed");
        exit(1);
    }

    std::cout << "RPC server started. Listening on port " << PORT << std::endl;

    while (true) {
        socklen_t clientAddressLength = sizeof(clientAddress);

        // Accept a new client connection
        if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength)) < 0) {
            perror("Error accepting client connection");
            exit(1);
        }

        std::cout << "New client connection accepted" << std::endl;

        // Perform the remote procedure call
        performRemoteProcedureCall(clientSocket);
    }

    close(serverSocket);
    return 0;
}
