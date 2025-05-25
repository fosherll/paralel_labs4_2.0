//
// Created by foshe on 15.05.2025.
//
#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <vector>
#include <thread>
#include <math.h>

#ifndef LAB_4_2_0_MESSAGE_H
#define LAB_4_2_0_MESSAGE_H
class Message {
private:
    std::vector<std::vector<int>> userMatrix;
    std::vector<int> userVector;
    int userNumThreads;
    double duration;
    char command = 'U';
    std::vector<int> result;
    int totalSize = 0;

public:
    std::vector<int> getResult()
    {
        return result;
    }
    std::vector<std::vector<int>> getUserMatrix()
    {
        return userMatrix;
    }
    std::vector<int> getUserVector()
    {
        return userVector;
    }
    int getUserNumThreads()
    {
        return userNumThreads;
    }
    double getDuration()
    {
        return duration;
    }
    char getCommand()
    {
        return command;
    }
    int getTotalsize()
    {
        return totalSize;
    }

    void setTotalSize() {
        this->totalSize = 1 + 4 +
                          4 + std::to_string(this->userNumThreads).size() +
                          4 + std::to_string(this->duration).size() +
                          4 + (this->userMatrix.size() * this->userMatrix.size() * sizeof(uint8_t))+
                          4 + (this->userVector.size() *sizeof(uint8_t))+
                          4 + (this->result.size()*sizeof(uint8_t));
    }

    void setResult(std::vector<int> &newResult)
    {
        result= newResult;
        this->setTotalSize();
    }
    void setUserMatrix(std::vector<std::vector<int>> &newMatrix)
    {
        this->userMatrix=newMatrix;
        this->setTotalSize();
    }
    void setUserVector(std::vector<int> &newVector)
    {
        this->userVector=newVector;
        this->setTotalSize();
    }
    void setUserNumThreads(int newNumThreads)
    {
        this->userNumThreads=newNumThreads;
        this->setTotalSize();
    }
    void setDuration(double newDuration)
    {
        this->duration=newDuration;
        this->setTotalSize();
    }
    void setCommand(char newCommnad)
    {
        this->command= newCommnad;
        this->setTotalSize();
    }

    char* encode()
    {
        std::string threadsStr = std::to_string(this->userNumThreads);
        std::string durationStr = std::to_string(this->duration);

        char* buffer = new char[this->totalSize];
        char* ptr = buffer;

        // Command
        *ptr = this -> command;
        ptr += 1;

        // Total message size
        uint32_t messageBodyTotalSize = htonl(this->totalSize - 5 );
        std::memcpy(ptr, &messageBodyTotalSize, sizeof(uint32_t));
        ptr += sizeof(uint32_t);

        // matrix size
        uint32_t sizeMatrix = htonl(userMatrix.size()*userMatrix.size()*sizeof(uint8_t));
        std::memcpy(ptr, &sizeMatrix, sizeof(uint32_t));
        ptr += sizeof(uint32_t);
        // matrix
        for (int i=0; i< userMatrix.size(); i++)
        {
            for (int j=0; j< userMatrix.size(); j++)
            {
                uint8_t MatrixNumberNet = static_cast<uint8_t>(userMatrix[i][j]);
                std::memcpy(ptr, &MatrixNumberNet, sizeof(uint8_t));
                ptr += sizeof(uint8_t);
            }
        }
        // vector size
        uint32_t sizeVector = htonl(userVector.size()*sizeof(uint8_t));
        std::memcpy(ptr, &sizeVector, 4);
        ptr += sizeof(uint32_t);
        // vector
        for (int i=0; i< userVector.size(); i++)
        {
            uint8_t vectorNumberNet = static_cast<uint8_t>(userVector[i]);
            std::memcpy(ptr, &vectorNumberNet, sizeof(uint8_t));
            ptr += sizeof(uint8_t);
        }

        // threads size
        uint32_t sizeThreads = htonl(threadsStr.size());
        std::memcpy(ptr, &sizeThreads, 4); ptr += 4;

        // threads
        std::memcpy(ptr, threadsStr.c_str(), threadsStr.size());
        ptr += threadsStr.size();

        // duration size
        uint32_t sizeDuration = htonl(durationStr.size());
        std::memcpy(ptr, &sizeDuration, 4); ptr += 4;
        // duration
        std::memcpy(ptr, durationStr.c_str(), durationStr.size());
        ptr += durationStr.size();

        // result size
        uint32_t sizeResult = htonl(result.size()*sizeof(uint8_t));
        std::memcpy(ptr, &sizeResult, 4); ptr += 4;
        // result
        for (int i=0; i< result.size(); i++)
        {
            uint8_t resultNumberNet = static_cast<uint8_t>(result[i]);
            std::memcpy(ptr, &resultNumberNet, sizeof(uint8_t));
            ptr += sizeof(uint8_t);
        }
        return buffer;
    }

    void decode(const char* buffer) {

        const char* ptr = buffer;
        // matrix size
        uint32_t sizeMatrix;
        std::memcpy(&sizeMatrix, ptr, sizeof (uint32_t));
        sizeMatrix = ntohl (sizeMatrix);
        ptr += 4;
        //std::cout<< "sizeMatrix =" << sizeMatrix << std::endl;

        // matrix
        int userMatrixSize = sqrt(static_cast<int>(sizeMatrix)/sizeof(uint8_t));
        //std::cout<< "userMatrixSize =" << userMatrixSize<<std::endl;
        this->userMatrix.resize(userMatrixSize, std::vector<int>(userMatrixSize)); // перезапись матрици которую пользователь передал
        for (int i = 0; i < userMatrixSize; ++i)
        {
            for (int j = 0; j < userMatrixSize; ++j)
            {
                uint8_t value;
                std::memcpy(&value, ptr, sizeof(uint8_t));
                //std::cout << "userMatrix after decoding: " << static_cast<int>(value) << std::endl;
                ptr += sizeof(uint8_t);
            }
        }

        // vector size
        uint32_t sizeVector;
        std::memcpy(&sizeVector, ptr, sizeof (uint32_t));
        sizeVector = ntohl (sizeVector);
        ptr += 4;
        //std::cout<< "sizeVector =" << sizeVector << std::endl;

        // vector
        int userVectorSize = static_cast<int>(sizeVector)/sizeof(uint8_t);
        //std::cout<< "userVectorSize =" << userVectorSize<<std::endl;
        this->userMatrix.resize(userVectorSize, std::vector<int>(userVectorSize)); // перезапись матрици которую пользователь передал
        for (int i = 0; i < userVectorSize; ++i)
        {
            uint8_t value;
            std::memcpy(&value, ptr, sizeof(uint8_t));
            //std::cout << "userVector after decoding: " << static_cast<int>(value) << std::endl;
            ptr += sizeof(uint8_t);
        }

        //Threads number
        uint32_t sizeThreads;
        std::memcpy(&sizeThreads, ptr, 4);
        sizeThreads = ntohl(sizeThreads);
        ptr += 4;
        //std::cout<< "sizeThreads =" << sizeThreads << std::endl;

        std::string threadsStr(ptr, sizeThreads);
        this->userNumThreads = std::stoi(threadsStr);
        ptr += sizeThreads;
        std::cout<< "userNumThreads =" << userNumThreads << std::endl;

        uint32_t sizeDuration;
        std::memcpy(&sizeDuration, ptr, 4);
        sizeDuration=ntohl(sizeDuration);
        ptr += 4;
        //std::cout<< "sizeDuration =" << sizeDuration << std::endl;

        std::string durationStr(ptr, sizeDuration);
        this->duration = std::stod(durationStr);
        ptr += sizeDuration;
        std::cout<< "duration =" << duration << std::endl;


        // result size
        uint32_t sizeResult;
        std::memcpy(&sizeResult, ptr, sizeof (uint32_t));
        sizeResult = ntohl (sizeResult);
        ptr += 4;
        //std::cout<< "sizeResult =" << sizeResult << std::endl;

        // result
        int userResultSize = static_cast<int>(sizeResult)/sizeof(uint8_t);
        //std::cout<< "userResultSize =" << userResultSize<<std::endl;
        this->userMatrix.resize(userResultSize, std::vector<int>(userResultSize)); // перезапись матрици которую пользователь передал
        for (int i = 0; i < userResultSize; ++i)
        {
            uint8_t value;
            std::memcpy(&value, ptr, sizeof(uint8_t));
            //std::cout << "userVector after decoding: " << static_cast<int>(value) << std::endl;
            ptr += sizeof(uint8_t);
        }
    }
};



#endif //LAB_4_2_0_MESSAGE_H
