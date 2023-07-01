#include <iostream>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctime>

#define BUFFER_SIZE 1024
using namespace std;

class RPCPacketHeader {
  public:
    // Request or Response.
    uint8_t messageType{0};

    // Length of Message
    uint32_t messageLength{0};

    // Procedure Number. Which procedure to Invoke on Server
    uint32_t procedureNumber{0};

    // Unique Identifier for Multiple Requests
    uint32_t requestID{0};

    // ProtocolVersion
    uint16_t protocolVersion{1};

    // Error in Response (if any)
    uint16_t errorCode{0};

    // Authentication Token to verify the authenticity of request
    uint64_t authenticationToken{123};

    // Timestamp
    uint64_t timestamp{0};

    // Prints Content of Packet Header for Debugging
    void printHeader()
    {
      cout << "Message Type: " << messageType << '\n';
      cout << "Message Length: " << messageLength << '\n';
      cout << "Procedure Number: " << procedureNumber << '\n';
      cout << "Message RequestID: " << requestID << '\n';
      cout << "Message ProtocolVersion: " << protocolVersion << '\n';
      cout << "Message ErrorCode: " << errorCode << '\n';
      cout << "Message AuthenticationToken: " << authenticationToken << '\n';
      cout << "Message TimeStamp: " << timestamp << '\n';
    }
};

class RPCPacket {
  public:
  // Stores the Authentication Token of server. Clients must provide this token in order to execute RPC.
  static uint64_t serverAuthorizationToken;

  // Generates a new requestID for each connection from a client.
  static uint64_t requestID;

  // Stores the list of Available Procedures that Server can handle.
  static vector<string> avalProcedures;

  // Header for RPC Packet
  RPCPacketHeader header;

  // Payload for RPC packet.
  string payload;


  // Constructor 
  RPCPacket(string payl = "")
  {
    payload = payl;
    header.messageType = 1;
    header.messageLength = payload.length();
    header.procedureNumber = 0;
    header.requestID = 1;
    header.protocolVersion = 1;
    header.errorCode = 0;
    header.authenticationToken = 123;
    header.timestamp = time(nullptr);
  }

  // Sends current packet through the clientSocket.
  bool sendPacket(int clientSocket)
  {
    // Encode the RPC Packet to be transmitted for security.
    encode();

    // Send the Packet Header
    char* headerptr = static_cast<char*>(static_cast<void*>(&header));
    int bytesWritten = write(clientSocket, headerptr, sizeof(header));
    if (bytesWritten < 0) {
        perror("Error writing to socket");
        close(clientSocket);
        exit(1);
    }

    // Send the Packet Payload
    bytesWritten = write(clientSocket, payload.c_str(), header.messageLength);
    if (bytesWritten < 0) {
        perror("Error writing to socket");
        close(clientSocket);
        exit(1);
    }

    return true;
  }

  // Recieves the RPC packet through the clientSocket. 
  bool recvPacket(int clientSocket)
  {
    char dataRead[BUFFER_SIZE];
    memset(dataRead, 0, BUFFER_SIZE);

    // Recieve the Packet Header
    int bytesRead = read(clientSocket, dataRead, sizeof(RPCPacketHeader));
    if (bytesRead < 0) {
        perror("Error reading from socket");
        close(clientSocket);
        exit(1);
    }
    header = *static_cast<RPCPacketHeader*>(static_cast<void*>(dataRead));


    // Recieve the Packet Payload
    bytesRead = read(clientSocket, dataRead, header.messageLength);
    if (bytesRead < 0) {
        perror("Error reading from socket");
        close(clientSocket);
        exit(1);
    }
    payload.assign(dataRead, dataRead+bytesRead);

    // Decode the RPC Packet after arrival.
    decode();

    return true;
  }

  // Genrerates a RPC Service Packet that specifies the list of services provided by the server.
  static RPCPacket getServiceMessage()
  {
    string payload{"Available Procedures\n"};
    int i{1};
    for(string& s: avalProcedures)
      payload += (to_string(i++)+". " + s + '\n');
    payload.append("Your Choice: ");
    RPCPacket service(payload);
    return service;
  }

  // Checks the authenticationToken of the Packet.
  bool checkAuthorizationToken()
  {    
    if(header.authenticationToken != RPCPacket::serverAuthorizationToken)
      return false;
    return true;
  }

  // Decodes the RPC packet.
  bool decode()
  {
      header.authenticationToken /= 12; 
      header.authenticationToken -= 232; 
      for(char& ch: payload)
        ch -= 13;
      return true;
  }

  // Encode the RPC Packet.
  bool encode()
  {
    header.authenticationToken += 232;
    header.authenticationToken *= 12;
    for(char& ch: payload)
      ch += 13;
  return true;
  }

};

// Initializes the requestID Number
uint64_t RPCPacket::requestID = 1;

// Sets the server's authorization Token
uint64_t RPCPacket::serverAuthorizationToken = 123;

// Initializes the List of Procedures available to the server.
vector<string> RPCPacket::avalProcedures = {"Procedure1","Procedure2", 
  "Procedure3", "Procedure4"};