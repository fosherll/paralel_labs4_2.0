#include <iostream>
#include "Task.h"
#include "ThreadPool.h"
#include <winsock2.h>
#include "Matrix.h"
#include "Message.h"
#include <vector>


SOCKET setUpSocket(int port, char *ip) {
    WSADATA wsaData;
    int wsInit = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsInit != 0) {
        std::cerr << "WSAStartup failed with error: " << wsInit << std::endl;
        return 0;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 0;
    }
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(ip);  // Replace with your desired IP address
    service.sin_port = htons(port);  // Choose a port number

    if (bind(serverSocket, reinterpret_cast<SOCKADDR *>(&service), sizeof(service)) == SOCKET_ERROR) {
        std::cout << "bind() failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 0;
    }

    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        std::cout << "listen(): Error listening on socket: " << WSAGetLastError() << std::endl;
    }

    return serverSocket;
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
        //std::cout<<"totalReceived length: "<<totalReceived<<std::endl;
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
    int taskId = 0;
    ThreadPool threadpool(4);

    SOCKET serverSocket = setUpSocket(8080, (char *) "127.0.0.1");
    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cout << "accept failed: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
        }
        else
        {
            Task task(taskId++, [clientSocket] {
                Matrix matrix;
                Message message;
                while (true) {
                    char command;
                    int received = recv(clientSocket, &command, 1, 0);
                    if (received <= 0)
                    {
                        break;
                    }

                    std::cout << "Received command: " << command << std::endl;

                     if (command=='M')
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
                        matrix.set(message.getUserMatrix(),message.getUserVector(),message.getUserNumThreads());
                        message.setCommand('C');
                        int sendReceive = send(clientSocket, message.encode(), message.getTotalsize(), 0);
                        if (sendReceive == SOCKET_ERROR) {
                            wprintf(L"send failed with error: %d\n", WSAGetLastError());
                            closesocket(clientSocket);
                            WSACleanup();
                            return 1;
                        }
                    }

                    else if (command=='P')
                    {
                        readMessage(clientSocket);
                        std::thread processingThread(&Matrix::multiplyMatrixVectorParallel, &matrix);
                        processingThread.detach();
                        message.setCommand('S');
                        int sendReceive = send(clientSocket, message.encode(), message.getTotalsize(), 0);
                        if (sendReceive == SOCKET_ERROR) {
                            wprintf(L"send failed with error: %d\n", WSAGetLastError());
                            closesocket(clientSocket);
                            WSACleanup();
                            return 1;
                        }
                    }
                    else if (command=='L')
                    {
                        readMessage(clientSocket);
                        message.setCommand(matrix.status());
                        int sendReceive = send(clientSocket, message.encode(), message.getTotalsize(), 0);
                        if (sendReceive == SOCKET_ERROR) {
                            wprintf(L"send failed with error: %d\n", WSAGetLastError());
                            closesocket(clientSocket);
                            WSACleanup();
                            return 1;
                        }
                    }
                     else if (command=='R')
                     {
                         readMessage(clientSocket);
                         message.setCommand('O');
                         message.setResult(matrix.getResult());
                         message.setDuration(matrix.getDuration());
                         message.setUserNumThreads(matrix.getNumThreads());
                         int sendReceive = send(clientSocket, message.encode(), message.getTotalsize(), 0);
                         if (sendReceive == SOCKET_ERROR)
                         {
                             wprintf(L"send failed with error: %d\n", WSAGetLastError());
                             closesocket(clientSocket);
                             WSACleanup();
                             return 1;
                         }
                     }
                    else if (command=='Q')
                    {
                        readMessage(clientSocket);
                        message.setCommand('B');
                        int sendReceive = send(clientSocket, message.encode(), message.getTotalsize(), 0);
                        closesocket(clientSocket);
                        break;
                    }
                    else
                    {
                        readMessage(clientSocket);
                        message.setCommand('u');
                        int sendReceive = send(clientSocket, message.encode(), message.getTotalsize(), 0);
                        if (sendReceive == SOCKET_ERROR) {
                            wprintf(L"send failed with error: %d\n", WSAGetLastError());
                            closesocket(clientSocket);
                            WSACleanup();
                            return 1;
                        }
                    };
                }
                return 0;
            });
            threadpool.addTask(task);
        }
    }
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