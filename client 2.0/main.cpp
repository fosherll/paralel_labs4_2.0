#include <iostream>
#include <winsock2.h>
#include <vector>
#include <ws2tcpip.h>
#include "Message.h"
#include <cstdlib>

SOCKET setUpSocket(int port, char *ip)
{
    WSADATA wsaData;
    int wsInit = WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET clientSocket = INVALID_SOCKET;

    if (wsInit != 0)
    {
        std::cerr << "WSAStartup failed with error: " << wsInit << std::endl;
        return 0;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cout<<"Socket creation failed"<<std::endl;
        return 0;
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (InetPtonA(AF_INET, ip, &addr.sin_addr) != 1)
    {
        std::cout<<"Invalid server IP"<<std::endl;
        return 0;
    }

    if (connect(clientSocket, (sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR)
    {
        std::cout<<"Connection failed"<<std::endl;
        return 0;
    }

    std::cout << "Connected to server.\n";
    return clientSocket;
}

void generateMatrix(int sizeMatrix,std::vector<std::vector<int>> &generatingMatrix)
{
    generatingMatrix.resize(sizeMatrix, std::vector<int>(sizeMatrix));
    for (int i=0;i<sizeMatrix; i++)
    {
        for(int j=0; j<sizeMatrix; j++)
        {
            generatingMatrix[i][j] = rand()%100;
        }
    }
}

void generateVector(int sizeVector,std::vector<int> &generatingVector)
{
    generatingVector.resize(sizeVector);
    for (int i=0;i<sizeVector; i++)
    {
        generatingVector[i] = rand()%100;
    }
}


void readMessage(SOCKET clientSocket)
{
    char receiveBufferMessageSize[4];
    int rbyteCount = recv(clientSocket, receiveBufferMessageSize, 4, 0);
    if (rbyteCount < 0)
    {
        return;
    }

    uint32_t messageLength;
    std::memcpy(&messageLength, receiveBufferMessageSize, 4);
    messageLength = int(ntohl(messageLength));

    char* receiveBufferMessage = new char[messageLength];
    int totalReceived = 0;

    while (totalReceived < messageLength)
    {
        std::cout<<"totalReceived length: "<<totalReceived<<std::endl;
        int rbyteCount = recv(clientSocket, receiveBufferMessage + totalReceived, messageLength - totalReceived, 0);
        if (rbyteCount <= 0)
        {
            delete[] receiveBufferMessage;
            break;
        }
        totalReceived += rbyteCount;
    }
}

int main() {

    SOCKET clientSocket = setUpSocket(8080, (char *) "127.0.0.1");
    while (true)
    {
        Message message;
        char command;
        std::cout<<"enter command: ";
        std::cin >> command;

        message.setCommand(command);

        if (command=='M')
        {
            int sizeMatrix;
            int userNumThreads;
            std::cout<<"enter size matrix: ";
            std::cin >> sizeMatrix;
            std::cout<<"enter thread pool: ";
            std::cin >> userNumThreads;

            std::vector<std::vector<int>> generatingMatrix;
            generateMatrix(sizeMatrix, generatingMatrix);
            std::vector<int> generatingVector;
            generateVector(sizeMatrix, generatingVector);

            message.setUserMatrix(generatingMatrix);
            message.setUserVector(generatingVector);
            message.setUserNumThreads(userNumThreads);

        }

        int sendReceive = send(clientSocket, message.encode(), message.getTotalsize(), 0);
        if (sendReceive == SOCKET_ERROR) {
            wprintf(L"send failed with error: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        char receiveBuffer[1];
        int rbyteCount = recv(clientSocket, receiveBuffer, 1, 0);
        if (rbyteCount < 0)
        {
            std::cout << "Server recv error: " << WSAGetLastError() << std::endl;
            break;
        }
        char serverAnswear = receiveBuffer[0];
        std::cout << "Received command: " << serverAnswear << std::endl;
        if (serverAnswear=='O')
        {
            char receiveBufferMessageSize[4];
            int rbyteCount = recv(clientSocket, receiveBufferMessageSize, 4, 0);
            if (rbyteCount < 0)
            {
                break;
            }

            uint32_t messageLength;
            std::memcpy(&messageLength, receiveBufferMessageSize, 4);
            messageLength = int(ntohl(messageLength));

            char* receiveBufferMessage = new char[messageLength];
            int totalReceived = 0;

            while (totalReceived < messageLength)
            {
                //std::cout<<"totalReceived length: "<<totalReceived<<std::endl;
                int rbyteCount = recv(clientSocket, receiveBufferMessage + totalReceived, messageLength - totalReceived, 0);
                if (rbyteCount <= 0)
                {
                    delete[] receiveBufferMessage;
                    break;
                }
                totalReceived += rbyteCount;
            }
            message.decode(receiveBufferMessage);
        }
        else
        {
            readMessage(clientSocket);
        }
    }
    return 0;
}
////Application protocol
//// client send M-> server take matrix
//// server send C(correct)
//// client send P -> server process matrix
//// server send S (process start)
//// client send L -> server process matrix
//// server send W(Wait) or I(in progress) or F(finish process) or D (already has data but waiting for start)
//// client send R -> result matrix
//// server send O (ready matrix)
//// client send Q (exit)
//// server send B (bye)
////client send unknown command
////server send U