# Planner

A weekly planner that uses Simulated Annealing (SA) to generate optimal study schedules, taking into account the non-linear nature of learning efficiency.

## Description
This program optimizes weekly study schedules by considering factors such as:
- Daily available time
- Subject-specific learning characteristics
- Diminishing returns due to mental fatigue
- Minimum effective study duration per subject

## Prerequisites
- C++ compiler
- Two configuration files:
  - `time.txt`
  - `matrix.txt`

## Installation
```bash
git clone https://github.com/Hojjat-Monzavi/Planning.git
cd Planning
g++ Planner.cpp -O3 -o planner
./planner
```

## Usage

### 1. Configure Available Time
Create `time.txt` with 7 numbers representing available study hours for each day of the week (Monday through Sunday).

Example:
```
2 3 2 4 3 5 1
```

### 2. Configure Subject Parameters
Create `matrix.txt` with 4 columns:

| Column | Description |
|--------|-------------|
| Weight | Multiplier for final learning score calculation |
| Min Time | Minimum required study session duration (hours) |
| Plateau | Point where productivity begins to decline (hours) |
| Name | Subject identifier for output display |

Example:
```
1.5  0.5  1.5 BASIC_MATHEMATICS
2.0  1.0  2.5 LOGIC_GATES_1
```

Note: After reaching the plateau time, productivity decreases by a factor of 10 (default) due to mental fatigue or content coverage completion.

## Technical Details
The program implements Simulated Annealing to optimize study schedules while considering:
- Non-linear learning efficiency
- Time constraints
- Subject-specific requirements
- Diminishing returns


## License
This software is released into the public domain. Anyone is free to copy, modify, publish, use, compile, or distribute this software.

## Author
Created by Hojjat Monzavi (@Diversitium)

Last updated: 20.12.2023
