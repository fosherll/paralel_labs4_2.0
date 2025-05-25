#include <iostream>
#include <vector>
#include <thread>

#ifndef LAB_4_2_0_MATRIX_H
#define LAB_4_2_0_MATRIX_H


class Matrix {
private:
    std::vector<std::vector<int>> matrix;
    std::vector<int> vector;
    std::vector<int> result;
    int numThreads;
    double duration =0.0;
    std::atomic<bool> finished = false;
    std::atomic<bool> inProgress = false;
    std::atomic<bool> wait = true;
public:
    std::vector<int> getResult()
    {
        if (finished)
        {
            return result;
        }
        std::vector<int> answear;
        return answear;
    }
    double getDuration()
    {
        if (finished)
        {
            return duration;
        }
        return 0.0;
    }
    int getNumThreads()
    {
            if (finished)
            {
                return numThreads;
            }
            return 0;
    };
    void set(std::vector<std::vector<int>> newMatrix,std::vector<int> newVector,int newNumThreads)
    {
        this->matrix=newMatrix;
        this->vector=newVector;
        this->numThreads=newNumThreads;
        result= std::vector<int>(newMatrix.size(), 0);
        wait= false;
        this->finished = false;
        this->inProgress = false;
        this->duration=0.0;
    }

    void multiplyMatrixVectorParallel() {
        if(!wait)
        {
            inProgress = true;
            auto start = std::chrono::high_resolution_clock::now();
            std::vector<std::thread> threads;
            int rowsPerThread = matrix.size() / numThreads;
            for (int t = 0; t < numThreads; ++t) {
                int startRow = t * rowsPerThread;
                int endRow = (t == numThreads - 1) ? matrix.size() : (startRow + rowsPerThread);
                threads.emplace_back([this, startRow, endRow]() {
                    for (int row = startRow; row < endRow; ++row) {
                        for (int j = 0; j < this->matrix[row].size(); ++j) {
                            this->result[row] += this->matrix[row][j] * this->vector[j];
                        }
                    }
                });
            }
            for (auto &t: threads)
            {
                if (t.joinable())
                {
                    t.join();
                }
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
            this->duration = duration.count() * 1e-9;
            inProgress = false;
            finished = true;
        }
    }

    char status()
    {
        if(wait)
        {
            return 'W';
        }
        if(inProgress)
        {
            return 'I';
        }
        if (!wait && !inProgress && !finished)
        {
            return 'D';
        }
        return 'F';
    }

};


#endif //LAB_4_2_0_MATRIX_H
