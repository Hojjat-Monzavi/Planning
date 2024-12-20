/*
Created by Hojjat Monzavi (@Diversitium)
Last updated: 20.12.2023

This is a (poorly developed) weekly planner that uses SA to return optimal plan considering the non-linear nature of studying.

Prerequisites:

    C++ compiler
    2 Files containing available time and subject's information

To build:
g++ Planner.cpp -O3 -o planner

Usage:

    Set up "time.txt":
    Enter 7 numbers representing available study hours for each day of the week
    Example: 2 3 2 4 3 5 1

    Configure "matrix.txt":
    File requires 4 columns:
        Column 1 (Weight): Multiplier used to calculate final learning score
        Column 2 (Min Time): Minimum duration required for a study session (in hours)
        Column 3 (Plateau): Point at which productivity declines substantially (in hours)
        Column 4 (Name): Text identifier used to display optimization results

    The program uses these values to calculate learning efficiency.
    After reaching Plateau Time, productivity decreases gradually
    (by default, reduces by factor of 10) due to mental fatigue
    or content coverage completion.

Example matrix.txt:
1.5 0.5 1.5 BASIC_MATHEMATICS
2.0 1.0 2.5 LOGIC_GATES_1

This software is released into the public domain.
No warranty of any kind is provided.

The author assumes no responsibility for any damages 
or losses arising from the use of this software.

Anyone is free to copy, modify, publish, use, compile, 
or distribute this software, for any purpose,
commercial or non-commercial, and by any means.
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <string>

int numTask;
int numDays = 7;
std::vector<float> dailyTimeLimit; // To store daily time limits
std::vector<std::string> taskNames; // To store task names

/**
 * Load the evaluation matrix from a file.
 */
void loadMatrix(std::vector<std::vector<float>>& matrix, const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile) {
        std::cerr << "Error opening file for reading!" << std::endl;
        return;
    }

    matrix.clear();
    taskNames.clear();
    std::vector<float> row(3);
    std::string taskName;
    while (inFile >> row[0] >> row[1] >> row[2] >> taskName) {
        matrix.push_back(row);
        taskNames.push_back(taskName);
    }

    inFile.close();
}

/**
 * Load time limits from file
 */
void loadTimeLimit(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile) {
        std::cerr << "Error opening time limit file for reading!" << std::endl;
        return;
    }

    dailyTimeLimit.clear();
    float limit;
    while (inFile >> limit) {
        dailyTimeLimit.push_back(limit);
    }

    // If fewer than 7 days specified, repeat the last value
    while (dailyTimeLimit.size() < numDays) {
        dailyTimeLimit.push_back(dailyTimeLimit.back());
    }

    inFile.close();
}

/**
 * Activation function
 */
float activation(float k1, float k2, float t1, float t) {
    if (t < k2) {
        return 0; 
    }else if (t<t1){
        return k1/t1*t;
    }else{
        return k1 + (t-t1)*k1/t1/10;
    }
}

/**
 * Evaluate the plan
 */
double getPlanEvaluation(const std::vector<std::vector<float>>& evaluationMatrix, 
                        const std::vector<std::vector<float>>& plan, 
                        int numTask) {
    double score = 0;
    double task_scor = 1;
    for (int taskIndex = 0; taskIndex < numTask; taskIndex++) {
        double task_sum = 0;
        for (int dayIndex = 0; dayIndex < numDays; dayIndex++) {
            score += activation(evaluationMatrix[taskIndex][0],
                              evaluationMatrix[taskIndex][1],
                              evaluationMatrix[taskIndex][2],
                              plan[taskIndex][dayIndex]);
            task_sum += plan[taskIndex][dayIndex]*evaluationMatrix[taskIndex][0];
        }
        task_scor += 1/(1+std::exp(-task_sum))/numTask;
    }
    return score * task_scor ;
}

/**
 * Print matrix with task names
 */
void printMatrixWithNames(const std::vector<std::vector<float>>& matrix, const std::string& caption) {
    if (matrix.empty() || matrix[0].empty()) {
        std::cout << "Empty matrix!" << std::endl;
        return;
    }

    const int rows = matrix.size();
    const int cols = matrix[0].size();

    // Find the maximum width needed for any number
    int maxWidth = 0;
    for (const auto& row : matrix) {
        for (const auto& elem : row) {
            int width = std::to_string(static_cast<long long>(elem)).length();
            if (elem != static_cast<long long>(elem)) {
                width = std::to_string(elem).length();
            }
            maxWidth = std::max(maxWidth, width);
        }
    }
    maxWidth += 2; // Add some padding

    int nameWidth = 0;
    for (const auto& name : taskNames) {
        nameWidth = std::max(nameWidth, static_cast<int>(name.length()));
    }

    // Print caption centered above the matrix
    if (!caption.empty()) {
        std::cout << std::string((nameWidth + maxWidth * cols + cols + 2 - caption.length()) / 2, ' ')
                  << caption << std::endl << std::endl;
    }

    // Print matrix contents with task names
    for (int i = 0; i < rows; i++) {
        std::cout << std::left << std::setw(nameWidth) << taskNames[i] << ": ";
        for (int j = 0; j < cols; j++) {
            std::cout << std::fixed << std::setprecision(0) 
                     << std::setw(maxWidth) << matrix[i][j] *60<< " ";
        }
        std::cout << std::endl;
    }
}

/**
 * Simulated Annealing optimization
 */
std::vector<std::vector<float>> simulatedAnnealing(
    const std::vector<std::vector<float>>& evaluationMatrix, 
    int maxIterations, 
    double initialTemp, 
    double coolingRate
) {
    // Initialize the current plan with random values
    std::vector<std::vector<float>> currentPlan(numTask, std::vector<float>(numDays));
    
    // Initialize with random values and adjust to meet time constraints
    for (int j = 0; j < numDays; j++) {
        float totalTime = 0;
        for (int i = 0; i < numTask; i++) {
            currentPlan[i][j] = static_cast<float>(rand()) / RAND_MAX;
            totalTime += currentPlan[i][j];
        }
        
        // Adjust values if total time exceeds limit
        if (totalTime > dailyTimeLimit[j]) {
            float scale = dailyTimeLimit[j] / totalTime;
            for (int i = 0; i < numTask; i++) {
                currentPlan[i][j] *= scale;
            }
        }
    }

    double currentScore = getPlanEvaluation(evaluationMatrix, currentPlan, numTask);
    std::vector<std::vector<float>> bestPlan = currentPlan;
    double bestScore = currentScore;
    double temperature = initialTemp;

    for (int iteration = 0; iteration < maxIterations; iteration++) {
        std::vector<std::vector<float>> newPlan = currentPlan;
        int taskIndex = rand() % numTask;
        int dayIndex = rand() % numDays;
        
        // Generate new value
        newPlan[taskIndex][dayIndex] += (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.3f;
        
        // Ensure value is non-negative
        if (newPlan[taskIndex][dayIndex] < 0) newPlan[taskIndex][dayIndex] = 0;
        
        // Check time constraint for the day
        float totalTime = 0;
        for (int i = 0; i < numTask; i++) {
            totalTime += newPlan[i][dayIndex];
        }
        
        // If time limit exceeded, scale back all tasks for that day
        if (totalTime > dailyTimeLimit[dayIndex]) {
            float scale = dailyTimeLimit[dayIndex] / totalTime;
            for (int i = 0; i < numTask; i++) {
                newPlan[i][dayIndex] *= scale;
            }
        }

        double newScore = getPlanEvaluation(evaluationMatrix, newPlan, numTask);
        double scoreDiff = newScore - currentScore;

        if (scoreDiff > 0 || (std::exp(scoreDiff / temperature) > static_cast<float>(rand()) / RAND_MAX)) {
            currentPlan = newPlan;
            currentScore = newScore;

            if (newScore > bestScore) {
                bestPlan = newPlan;
                bestScore = newScore;
            }
        }

        temperature *= coolingRate;

        if (iteration % 100 == 0 || iteration == maxIterations - 1) {
            std::cout << "Iteration " << iteration << ", Best Score: " << bestScore 
                     << ", Current Score: " << currentScore << ", Temperature: " << temperature << std::endl;
        }
    }

    return bestPlan;
}

int main() {
    srand(time(NULL));

    // Load evaluation matrix
    std::vector<std::vector<float>> evaluationMatrix;
    std::string matrixFilename = "matrix.txt";
    std::string timeFilename = "time.txt";

    loadMatrix(evaluationMatrix, matrixFilename);
    loadTimeLimit(timeFilename);

    // Check if data was loaded successfully
    if (evaluationMatrix.empty()) {
        std::cerr << "Error: No tasks loaded from the file." << std::endl;
        return 1;
    }

    if (dailyTimeLimit.empty()) {
        std::cerr << "Error: No time limits loaded from the file." << std::endl;
        return 1;
    }

    numTask = evaluationMatrix.size();

    // Parameters for Simulated Annealing
    int maxIterations = 10000000;
    double initialTemp = 1000.0;
    double coolingRate = 0.999998;

    // Perform optimization
    std::vector<std::vector<float>> optimizedPlan = simulatedAnnealing(
        evaluationMatrix, maxIterations, initialTemp, coolingRate);

    // Print results
    double finalScore = getPlanEvaluation(evaluationMatrix, optimizedPlan, numTask);
std::cout << "\nOptimized Plan Evaluation Score: " << finalScore << std::endl;

    // Print the optimized plan with task names
    printMatrixWithNames(optimizedPlan, "Final Optimized Plan");

    // Print daily totals and limits
    std::cout << "\nDaily Totals vs Limits:" << std::endl;
    for (int j = 0; j < numDays; j++) {
        float totalTime = 0;
        for (int i = 0; i < numTask; i++) {
            totalTime += optimizedPlan[i][j];
        }
        std::cout << "Day " << j + 1 << ": Total = " << totalTime 
                  << ", Limit = " << dailyTimeLimit[j] << std::endl;
    }

    return 0;
}
