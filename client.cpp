#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 4000
#define BUFFER_SIZE 1024

void performRemoteProcedureCall(int clientSocket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Send the RPC request to the server
    const char* request = "RPC request";
    int bytesWritten = write(clientSocket, request, strlen(request));
    if (bytesWritten < 0) {
        perror("Error writing to socket");
        close(clientSocket);
        exit(1);
    }

    // Receive the RPC response from the server
    int bytesRead = read(clientSocket, buffer, BUFFER_SIZE);
    if (bytesRead < 0) {
        perror("Error reading from socket");
        close(clientSocket);
        exit(1);
    }

    std::cout << "Received RPC response: " << buffer << std::endl;

    close(clientSocket);
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Prepare the server address structure
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Connection to server failed");
        exit(1);
    }

    std::cout << "Connected to RPC server" << std::endl;

    // Perform the remote procedure call
    performRemoteProcedureCall(clientSocket);

    close(clientSocket);
    return 0;
}
