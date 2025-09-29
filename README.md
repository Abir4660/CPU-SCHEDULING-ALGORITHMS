# CPU Scheduling Algorithm Simulator (C++)

## 📌 Overview
This project is a **console-based CPU Scheduling Algorithm Simulator** written in **C++**.  
It provides implementations of classic CPU scheduling algorithms used in Operating Systems:

- **First-Come-First-Serve (FCFS)**  
- **Shortest Job First (SJF, non-preemptive)**  
- **Priority Scheduling (non-preemptive)**  
- **Round Robin (RR, preemptive with time quantum)**  

The program allows users to:
- Input or load process data  
- Visualize scheduling results with **Gantt charts**  
- Compute **average waiting time** and **turnaround time**  
- Compare algorithms on the same workload  

---

## 🏗️ Features
- 📋 Input custom process data or load sample dataset  
- 🖥️ Menu-driven interface for easy navigation  
- ⏳ Multiple scheduling algorithms in one program  
- 📊 **Gantt chart** display of process execution  
- 📈 Average Waiting Time (AWT) & Average Turnaround Time (TAT) metrics  
- ⚡ Algorithm comparison module to find the most efficient one for a workload  

---

## 📂 Project Structure
main.cpp # Single-file program with all algorithms
README.md # Project documentation


---

## ⚙️ Compilation & Execution

### Using G++ (Linux/Mac/Windows with MinGW):
```bash
g++ main.cpp -o scheduler
./scheduler
or,
Using Code::Blocks or Dev-C++:
Create a new Console Project.
Replace the main.cpp with this file.
Build & Run.

🖥️ Example Menu
CPU Scheduling Algorithm Simulator and Evaluator (C++)

Tip: Enter your own processes via option 1. You can also try sample data via option 2.

================== MAIN MENU ==================
1) Enter processes
2) Load sample data
3) Set Round Robin quantum (current: 2)
4) Run FCFS
5) Run SJF (Non-Preemptive)
6) Run Priority (Non-Preemptive)
7) Run Round Robin
8) Compare all algorithms
9) Exit

🖼️ Sample Output (FCFS Example)
================== FCFS ==================

+-------+----+----+----+
|  P1   | P2 | P3 | P4 |
+-------+----+----+----+
0       7    11   12   16

PID     Arr       Burst     Prio      Start      Finish     Waiting    Turnaround
--------------------------------------------------------------------------------------
P1      0         7         2         0          7          0          7
P2      2         4         3         7          11         5          9
P3      4         1         1         11         12         7          8
P4      5         4         4         12         16         7          11
--------------------------------------------------------------------------------------
Average Waiting Time: 4.75
Average Turnaround Time: 8.75

🛠️ Tech Stack
Language: C++ (C++11 compatible)
IDE Recommended: Code::Blocks (MinGW/GCC)
Paradigm: Procedural with modular functions

🤝 Contribution
Fork this repository
Create a feature branch (feature/new-algorithm)
Commit your changes
Push to your fork
Open a Pull Request

📜 License
This project is licensed under the MIT License.

## 📊 Diagrams

For better understanding:
- [Flowchart of Program Execution](diagram.png)
- [UML Class Diagram](docs/uml.png)

👨‍💻 Author

Developed by MD Abir Ahmed
📧 Email: abbir778@gmail.com
🌐 GitHub: Abir4660
