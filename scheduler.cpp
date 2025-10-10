#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <sstream>
#include <string>

using namespace std;

// We keep track of where we are in the instruction list and how long we're blocked for.
struct Process {
    int pid;
    int priority;
    vector<string> instructions;
    size_t current_instruction = 0;
    int blocked_cycles = 0;

    Process(int id, int p) : pid(id), priority(p) {}

    // Helper functions to manage process instructions
    bool hasMoreInstructions() const {
        return current_instruction < instructions.size();
    }
    string getCurrentInstruction() {
        if (hasMoreInstructions()) {
            return instructions[current_instruction++];
        }
        return "";  // Process is done when we hit empty string
    }
};

// Custom comparator for our priority queue - higher priority processes get to run first
struct ComparePriority {
    bool operator()(Process* a, Process* b) {
        return a->priority < b->priority;
    }
};

// Keeping track of everything happening in both console and log file
void logEvent(ofstream& logFile, const string& event) {
    cout << event << endl;
    logFile << event << endl;
}

int main(int argc, char* argv[]) {
    // Making sure we get the number of processes as a command line arg
    if (argc != 2) {
        cerr << "Usage: ./proj03 N" << endl;
        return 1;
    }
    int numProcesses = stoi(argv[1]);
    // - readyQueue: processes ready to run 
    // - blockedProcesses: processes waiting for I/O or network
    // - runningProcess: the process currently executing
    priority_queue<Process*, vector<Process*>, ComparePriority> readyQueue;
    vector<Process*> blockedProcesses;
    Process* runningProcess = nullptr;
    int timer = 0;
    ofstream logFile("LOG.txt");

    // Loading all process files and their instructions
    // Each file has priority on first line, then instructions
    for (int i = 1; i <= numProcesses; i++) {
        string filename = "process" + to_string(i);
        ifstream file(filename);
        
        if (!file) {
            cerr << "Error: Could not open " << filename << endl;
            continue;
        }
        int priority;
        file >> priority;
        
        Process* proc = new Process(i, priority);
        string line;
        getline(file, line); // Clear the priority line

        while (getline(file, line)) {
            proc->instructions.push_back(line);
        }
        
        readyQueue.push(proc);
        file.close();
    }
    // Main scheduling loop - keep going until all processes are done
    while (!readyQueue.empty() || !blockedProcesses.empty() || runningProcess != nullptr) {
        timer++;

        // Handle blocked processes
        for (auto it = blockedProcesses.begin(); it != blockedProcesses.end();) {
            Process* proc = *it;
            if (--proc->blocked_cycles <= 0) {
                logEvent(logFile, "Process " + to_string(proc->pid) + ": Blocked -> Ready");
                readyQueue.push(proc);
                it = blockedProcesses.erase(it);
            } else {
                ++it;
            }
        }
        // Timer interrupt slice expired (every 5 cycles)
        if (runningProcess && timer % 5 == 0) {
            logEvent(logFile, "Hardware Interrupt: Timer Interval");
            logEvent(logFile, "Process " + to_string(runningProcess->pid) + ": Running -> Ready");
            readyQueue.push(runningProcess);
            runningProcess = nullptr;
        }

        // If nothing's running, grab highest priority process from ready queue
        if (runningProcess == nullptr && !readyQueue.empty()) {
            runningProcess = readyQueue.top();
            readyQueue.pop();
            logEvent(logFile, "Process " + to_string(runningProcess->pid) + ": Ready -> Running");
        }

        // Execute current process
        if (runningProcess != nullptr) {
            string instruction = runningProcess->getCurrentInstruction();
            // Process finished without explicit TERMINATE
            if (instruction.empty()) {
                logEvent(logFile, "Process " + to_string(runningProcess->pid) + ": Running -> Halted");
                delete runningProcess;
                runningProcess = nullptr;
                timer = 0;
                continue;
            }
            // Handling system calls (TERMINATE, ERROR, NETWORK)
            if (instruction.find("SYS_CALL") != string::npos) {
                istringstream iss(instruction);
                string syscall, type;
                iss >> syscall >> type;
                
                if (type == "TERMINATE") {
                    logEvent(logFile, "Software Interrupt: TERMINATE");
                    logEvent(logFile, "Process " + to_string(runningProcess->pid) + ": Running -> Halted");
                    delete runningProcess;
                    runningProcess = nullptr;
                }
                else if (type.find("ERROR") != string::npos) {
                    logEvent(logFile, "Software Interrupt: ERROR Runtime Error");
                    logEvent(logFile, "Process " + to_string(runningProcess->pid) + ": Running -> Halted");
                    delete runningProcess;
                    runningProcess = nullptr;
                }
                else {
                    // Handling NETWORK syscall
                    string network;
                    int cycles;
                    iss >> network >> cycles;
                    
                    logEvent(logFile, "Software Interrupt: NETWORK " + to_string(cycles));
                    logEvent(logFile, "Process " + to_string(runningProcess->pid) + ": Running -> Blocked");
                    runningProcess->blocked_cycles = cycles;
                    blockedProcesses.push_back(runningProcess);
                    runningProcess = nullptr;
                }
                timer = 0;  // Resetting timer after any system call
            }
        }
    }

    logFile.close();
    return 0;
}