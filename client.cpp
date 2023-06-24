#include <iostream>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "RPC.cpp"

#define SERVER_PORT 9000
#define BUFFER_SIZE 1024
using namespace std;

// Returns current time formatted in "Day-Month-Year Hour:Minute:Second"
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

  // Get the List of services available to the Server.
  RPCPacket services;
  services.recvPacket(clientSocket);

  // Get User Choice for Remote Procedure to Call
  cout << services.payload << ": ";
  int ch; cin >> ch;


  // Get the AuthorizationToken of Server from User.
  uint64_t authorizationToken;
  cout << getTime() << ": Enter AuthorizationToken For Server: ";
  cin >> authorizationToken;

  // Create a RPC Request Packet for Procedure Call.
  RPCPacket request;
  request.header.messageType = 2;
  request.header.messageLength = 0;
  request.header.requestID  = RPCPacket::requestID++;
  request.header.authenticationToken = authorizationToken;
  request.header.procedureNumber = ch;
  request.header.timestamp = time(nullptr);


  // Send the Procedure Exectution Request Packet to the Server.
  request.sendPacket(clientSocket);
  
  // Get the result of Execution of Procedure from the Server.
  RPCPacket result;
  result.recvPacket(clientSocket);

  // Check if there was some error while execution of Procedure on the Server.
  if(result.header.errorCode == 0)
    cout << getTime() << ": Result of Execution of " << RPCPacket::avalProcedures[ch-1] << " on Server:\n" << result.payload << '\n';
  else
    cout << getTime() << ": " << result.payload << '\n'; 
  return;
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;

    // Get the server IP address from user.
    string serverIP;
    cout << getTime() << ": Enter Server IP Address: ";
    cin >> serverIP;

    // Prepare the server address structure
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(serverIP.c_str());
    serverAddress.sin_port = htons(SERVER_PORT);

    while(true)
    {

      // Create socket
      if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
          perror("Socket creation failed");
          exit(1);
      }

      // Connect to the server
      if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
          perror("Connection to server failed");
          exit(1);
      }

      std::cout << getTime() << ": Connected to RPC server" << std::endl;

      // Perform the remote procedure call
      performRemoteProcedureCall(clientSocket);
      close(clientSocket);

      // Whether client wants to perfrom some mroe procedure calls.
      cout << "\nDo You Want to Continue? (y/n): ";
      char ch; cin >> ch;
      if(ch == 'n' || ch == 'N')
        break;
    }

    return 0;
}
