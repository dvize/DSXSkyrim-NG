#include <WinSock2.h>
#include "UDPHelper.h"

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

#define SERVER "127.0.0.1"  // or "localhost" - ip address of UDP server
#define BUFLEN 512          // max length of answer
#define PORT 6969           // the port on which to listen for incoming data

using socket_t = decltype(socket(0, 0, 0));

namespace DSXSkyrim 
{
    extern socket_t mysocket;
    extern sockaddr_in server;

    void sendToDSX(std::string& s) {
        // convert json to string and then char array

        char message[512];
        strcpy(message, s.c_str());
        for (int i = s.size(); i < 512; i++) {
            message[i] = '\0';
        }

        // send the message

        if (sendto(mysocket, message, strlen(message), 0, (sockaddr*)&server, sizeof(sockaddr_in)) == SOCKET_ERROR) {
            logger::error("sendtodsx() failed with error code: %d", WSAGetLastError());
        }

        // lastSend1 = s;
    }

    void StartupUDP() {
        // initialise winsock
        WSADATA ws;
        logger::info("Initialising Winsock...");
        if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
            logger::error("Failed. Error Code: %d", WSAGetLastError());
        }
        logger::info("Initialised.\n");
        // create socket

        if ((mysocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)  // <<< UDP socket
        {
            logger::error("socket() failed with error code: %d", WSAGetLastError());
        }

        // setup address structure
        memset((char*)&server, 0, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);
        server.sin_addr.S_un.S_addr = inet_addr(SERVER);
    }


}
