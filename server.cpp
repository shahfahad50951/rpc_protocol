#include <iostream>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include "RPC.cpp"
#include "procedures.cpp"

using namespace std;
#define SERVER_PORT 9000 

// Returs the current time formatted in "Day-Month-Year Hour:Minute:Second"
string getTime()
{
  string str;
  time_t timer;
  char buffer[26];
  struct tm* tm_info;

  timer = time(nullptr);
  tm_info = localtime(&timer);

  strftime(buffer, 26, "%d-%m-%Y %H:%M:%S", tm_info);
  for(int i{0}; buffer[i] != 0; i++)
    str.push_back(buffer[i]);
  return str;
}

void performRemoteProcedureCall(int clientSocket) {

  // Advertise the Services to the Client
    RPCPacket services = RPCPacket::getServiceMessage();
    services.sendPacket(clientSocket);

    // Take the response from client.
    RPCPacket response;
    response.recvPacket(clientSocket);

    // Check the authorization Token
    if(!response.checkAuthorizationToken())
    {
      cout << getTime() << ": Invalid AuthorizationToken By Client " << clientSocket << '\n';
      RPCPacket error{"Invalid AuthorizationToken"};
      error.header.errorCode = 1;
      error.sendPacket(clientSocket);
      return;
    }


    // Execute the Procedure requested by the client.
    int procedure = response.header.procedureNumber;
    int error = 0;
    string output;
    switch(procedure)
    {
      case 1:
        output = procedure1();
        break;
      case 2:
        output = procedure2();
        break;
      case 3:
        output = procedure3();
        break;
      case 4:
        output = procedure4();
        break;
      default:
        cout << getTime() << ": Invalid Procedure Invoked By Client: " << clientSocket << '\n';
        output = "Invalid Procedure Number";
        error = 2;
        break;
    }

    // Send the output of executed procedure to the client.
    RPCPacket result{output};
    result.header.messageType = 1;
    result.header.errorCode = error;
    result.sendPacket(clientSocket);

    // Close the connection with the client.
    close(clientSocket);
    cout << getTime() << ": Terminating Client Connection...\n";
    return;
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;

    // Create Server socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Prepare the server address structure
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);

    // Bind the socket to the specified address and SERVER_PORT
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Socket binding failed");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 3) < 0) {
        perror("Socket listening failed");
        exit(1);
    }

    std::cout << getTime() << ": RPC server started. Listening on Server Port: " << SERVER_PORT << std::endl;

    while (true) {
        socklen_t clientAddressLength = sizeof(clientAddress);

        // Accept a new client connection
        if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength)) < 0) {
            perror("Error accepting client connection");
            exit(1);
        }

        std::cout << getTime() << ": New client connection accepted" << std::endl;

        // Perform the remote procedure call 
        performRemoteProcedureCall(clientSocket);
    }

    // Terminate the Server
    close(serverSocket);
    return 0;
}
