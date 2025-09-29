// CPU Scheduling Algorithm Simulator and Evaluator
// Works in Code::Blocks (GCC/MinGW) — single-file console program
// Algorithms: FCFS, SJF (non-preemptive), Round Robin (preemptive), Priority (non-preemptive)
// Features: Menu interface, Gantt chart, average waiting time/turnaround, input validation,
//           comparison module, simple report-like outputs per run

#include <bits/stdc++.h>
using namespace std;

struct Process {
    int pid;            // numeric ID (1..N)
    string name;        // optional label like P1
    int arrival;        // arrival time >= 0
    int burst;          // CPU burst time > 0
    int priority;       // lower value = higher priority (documented)
};

struct GanttSegment {
    string label;       // e.g., P1 or IDLE
    int start;          // inclusive
    int end;            // exclusive (time just after segment)
};

struct Result {
    vector<GanttSegment> timeline;               // Gantt segments in order
    unordered_map<int,int> completionTime;       // pid -> completion time
    unordered_map<int,int> startTime;            // pid -> first start time
    unordered_map<int,int> turnaround;           // pid -> TAT
    unordered_map<int,int> waiting;              // pid -> Waiting time
    double avgWaiting = 0.0;
    double avgTurnaround = 0.0;
};

// Utility: pretty print Gantt chart
static void printGantt(const vector<GanttSegment>& tl){
    if(tl.empty()){ cout << "(No timeline generated)\n"; return; }
    // top bar
    for(const auto &seg: tl){
        cout << "+" << string(max(1, seg.end - seg.start), '-');
    }
    cout << "+\n";
    // labels
    for(const auto &seg: tl){
        int w = max(1, seg.end - seg.start);
        string lab = seg.label;
        if((int)lab.size() > w) lab = lab.substr(0, w);
        int padLeft = (w - (int)lab.size())/2;
        int padRight = w - (int)lab.size() - padLeft;
        cout << "|" << string(padLeft, ' ') << lab << string(padRight, ' ');
    }
    cout << "|\n";
    // bottom bar
    for(const auto &seg: tl){
        cout << "+" << string(max(1, seg.end - seg.start), '-');
    }
    cout << "+\n";
    // time marks
    int t = 0;
    if(!tl.empty()) t = tl.front().start;
    cout << t;
    for(const auto &seg: tl){
        int w = max(1, seg.end - seg.start);
        cout << string(w, ' ') << seg.end;
    }
    cout << "\n";
}

// Helper to compute waiting/turnaround given completion times
static Result finalizeMetrics(const vector<Process>& procs, Result res){
    long long sumW = 0, sumT = 0;
    for(const auto& p: procs){
        int ct = res.completionTime[p.pid];
        int tat = ct - p.arrival; // turnaround
        int wt = tat - p.burst;   // waiting
        if(wt < 0) wt = 0;        // guard if idle before arrival
        res.turnaround[p.pid] = tat;
        res.waiting[p.pid] = wt;
        sumW += wt; sumT += tat;
    }
    int n = (int)procs.size();
    res.avgWaiting = n ? (double)sumW / n : 0.0;
    res.avgTurnaround = n ? (double)sumT / n : 0.0;
    return res;
}

// FCFS with arrival times (non-preemptive)
static Result runFCFS(vector<Process> procs){
    Result res;
    sort(procs.begin(), procs.end(), [](const Process&a, const Process&b){
        if(a.arrival != b.arrival) return a.arrival < b.arrival;
        return a.pid < b.pid;
    });
    int t = 0;
    for(const auto &p: procs){
        if(t < p.arrival){
            // idle gap
            res.timeline.push_back({"IDLE", t, p.arrival});
            t = p.arrival;
        }
        res.startTime[p.pid] = t;
        res.timeline.push_back({p.name, t, t + p.burst});
        t += p.burst;
        res.completionTime[p.pid] = t;
    }
    return finalizeMetrics(procs, res);
}

// SJF Non-preemptive with arrival times
static Result runSJF(vector<Process> procs){
    Result res;
    int n = (int)procs.size();
    vector<bool> done(n, false);
    // map pid->index
    unordered_map<int,int> idx;
    for(int i=0;i<n;i++) idx[procs[i].pid]=i;

    int t = 0, finished = 0;
    // sort by arrival to know earliest arrival
    vector<Process> byArr = procs;
    sort(byArr.begin(), byArr.end(), [](const Process&a,const Process&b){
        if(a.arrival!=b.arrival) return a.arrival<b.arrival; return a.pid<b.pid;
    });
    size_t nextA = 0;
    // Use a priority queue picking smallest burst among arrived
    struct Node{int burst, arrival, pid; string name;};
    auto cmp = [](const Node&a, const Node&b){
        if(a.burst!=b.burst) return a.burst > b.burst; // min-heap by burst
        if(a.arrival!=b.arrival) return a.arrival > b.arrival;
        return a.pid > b.pid;
    };
    priority_queue<Node, vector<Node>, decltype(cmp)> pq(cmp);

    if(!byArr.empty()) t = min(t, byArr[0].arrival);

    while(finished < n){
        while(nextA < byArr.size() && byArr[nextA].arrival <= t){
            auto &p = byArr[nextA++];
            pq.push({p.burst, p.arrival, p.pid, p.name});
        }
        if(pq.empty()){
            // jump to next arrival
            if(nextA < byArr.size()){
                int old = t; t = max(t, byArr[nextA].arrival);
                if(old < t) res.timeline.push_back({"IDLE", old, t});
                continue;
            } else break; // nothing to schedule
        }
        auto cur = pq.top(); pq.pop();
        int i = idx[cur.pid];
        res.startTime[cur.pid] = t;
        res.timeline.push_back({procs[i].name, t, t + procs[i].burst});
        t += procs[i].burst;
        res.completionTime[cur.pid] = t;
        done[i] = true; finished++;
    }
    return finalizeMetrics(procs, res);
}

// Priority Non-preemptive (lower priority value = higher priority)
static Result runPriorityNP(vector<Process> procs){
    Result res;
    int n = (int)procs.size();
    unordered_map<int,int> idx; for(int i=0;i<n;i++) idx[procs[i].pid]=i;
    vector<bool> done(n,false);

    vector<Process> byArr = procs;
    sort(byArr.begin(), byArr.end(), [](const Process&a,const Process&b){
        if(a.arrival!=b.arrival) return a.arrival<b.arrival; return a.pid<b.pid;
    });
    int t = 0; size_t nextA = 0; int finished = 0;
    struct Node{int priority, arrival, pid; string name; int burst;};
    auto cmp = [](const Node&a,const Node&b){
        if(a.priority!=b.priority) return a.priority > b.priority; // min priority value first
        if(a.arrival!=b.arrival) return a.arrival > b.arrival;
        return a.pid > b.pid;
    };
    priority_queue<Node, vector<Node>, decltype(cmp)> pq(cmp);

    if(!byArr.empty()) t = min(t, byArr[0].arrival);

    while(finished < n){
        while(nextA < byArr.size() && byArr[nextA].arrival <= t){
            auto &p = byArr[nextA++];
            pq.push({p.priority, p.arrival, p.pid, p.name, p.burst});
        }
        if(pq.empty()){
            if(nextA < byArr.size()){
                int old = t; t = max(t, byArr[nextA].arrival);
                if(old < t) res.timeline.push_back({"IDLE", old, t});
                continue;
            } else break;
        }
        auto cur = pq.top(); pq.pop();
        res.startTime[cur.pid] = t;
        res.timeline.push_back({cur.name, t, t + cur.burst});
        t += cur.burst;
        res.completionTime[cur.pid] = t;
        done[idx[cur.pid]] = true; finished++;
    }
    return finalizeMetrics(procs, res);
}

// Round Robin (preemptive) with arrival times
static Result runRR(vector<Process> procs, int quantum){
    Result res;
    if(quantum <= 0) quantum = 1; // safety

    // sort by arrival
    vector<Process> byArr = procs;
    sort(byArr.begin(), byArr.end(), [](const Process&a,const Process&b){
        if(a.arrival!=b.arrival) return a.arrival<b.arrival; return a.pid<b.pid;
    });

    // remaining burst
    unordered_map<int,int> rem;
    unordered_map<int,string> nameOf;
    for(const auto &p: procs){ rem[p.pid]=p.burst; nameOf[p.pid]=p.name; }

    queue<int> q; // pid queue
    int t = 0; size_t nextA = 0; int finished = 0; int n = (int)procs.size();

    auto enqueueArrived = [&](){
        while(nextA < byArr.size() && byArr[nextA].arrival <= t){
            q.push(byArr[nextA].pid);
            nextA++;
        }
    };

    if(!byArr.empty()) t = min(t, byArr[0].arrival);

    enqueueArrived();

    while(finished < n){
        if(q.empty()){
            // CPU idle until next arrival
            if(nextA < byArr.size()){
                int old = t; t = max(t, byArr[nextA].arrival);
                if(old < t) res.timeline.push_back({"IDLE", old, t});
                enqueueArrived();
                continue;
            } else break;
        }
        int pid = q.front(); q.pop();
        if(rem[pid] == 0) continue; // skip finished
        // record first start if not present
        if(!res.startTime.count(pid)) res.startTime[pid] = t;

        int slice = min(quantum, rem[pid]);
        int start = t; int end = t + slice;
        res.timeline.push_back({nameOf[pid], start, end});
        rem[pid] -= slice; t = end;

        // add newly arrived during this time
        enqueueArrived();

        if(rem[pid] > 0){
            q.push(pid); // requeue
        } else {
            res.completionTime[pid] = t;
            finished++;
        }
    }

    return finalizeMetrics(procs, res);
}

// Input helpers and validation
static int askInt(const string& prompt, int minVal, int maxVal){
    while(true){
        cout << prompt;
        string line; if(!getline(cin, line)) { cin.clear(); continue; }
        stringstream ss(line);
        int x; char c;
        if(!(ss >> x) || (ss >> c)){
            cout << "Invalid input. Please enter an integer.\n"; continue;
        }
        if(x < minVal || x > maxVal){
            cout << "Value out of range (" << minVal << ".." << maxVal << "). Try again.\n"; continue;
        }
        return x;
    }
}

static vector<Process> readProcesses(){
    int n = askInt("Enter number of processes (1..50): ", 1, 50);
    vector<Process> v; v.reserve(n);
    cout << "\nNOTE: Lower priority value means HIGHER priority. All times are in arbitrary units.\n";
    for(int i=1;i<=n;i++){
        cout << "\n--- Process " << i << " ---\n";
        int arrival = askInt("Arrival time (>=0): ", 0, 1e9);
        int burst = askInt("Burst time (>0): ", 1, 1e9);
        int prio = askInt("Priority (0 = highest, 100 = lowest typical): ", 0, 1000000000);
        Process p; p.pid = i; p.name = string("P") + to_string(i); p.arrival = arrival; p.burst = burst; p.priority = prio;
        v.push_back(p);
    }
    return v;
}

static void printTable(const vector<Process>& procs, const Result& r){
    cout << left << setw(8) << "PID" << setw(10) << "Arr" << setw(10) << "Burst" << setw(10) << "Prio"
         << setw(12) << "Start" << setw(12) << "Finish" << setw(12) << "Waiting" << setw(12) << "Turnaround" << "\n";
    cout << string(86,'-') << "\n";
    for(const auto &p: procs){
        int st = r.startTime.count(p.pid)? r.startTime.at(p.pid) : -1;
        int ft = r.completionTime.count(p.pid)? r.completionTime.at(p.pid) : -1;
        int wt = r.waiting.count(p.pid)? r.waiting.at(p.pid) : -1;
        int tat= r.turnaround.count(p.pid)? r.turnaround.at(p.pid) : -1;
        cout << left << setw(8) << p.name << setw(10) << p.arrival << setw(10) << p.burst << setw(10) << p.priority
             << setw(12) << st << setw(12) << ft << setw(12) << wt << setw(12) << tat << "\n";
    }
    cout << string(86,'-') << "\n";
    cout << fixed << setprecision(2);
    cout << "Average Waiting Time: " << r.avgWaiting << "\n";
    cout << "Average Turnaround Time: " << r.avgTurnaround << "\n";
}

static void runAndShow(const string& title, const vector<Process>& procs, function<Result(vector<Process>)> fn){
    cout << "\n================== " << title << " ==================\n";
    Result r = fn(procs);
    printGantt(r.timeline);
    printTable(procs, r);
}

static void runAndShowRR(const string& title, const vector<Process>& procs, int quantum){
    cout << "\n================== " << title << " (Q=" << quantum << ") ==================\n";
    Result r = runRR(procs, quantum);
    printGantt(r.timeline);
    printTable(procs, r);
}

static void compareAll(const vector<Process>& procs, int quantum){
    struct Item{string name; Result res;};
    vector<Item> items;
    items.push_back({"FCFS", runFCFS(procs)});
    items.push_back({"SJF (Non-Preemptive)", runSJF(procs)});
    items.push_back({"Priority (Non-Preemptive)", runPriorityNP(procs)});
    items.push_back({string("Round Robin (Q=")+to_string(quantum)+")", runRR(procs, quantum)});

    cout << "\n========= Comparison (by Average Waiting Time) =========\n";
    double best = 1e100; string bestName;
    for(const auto &it: items){
        cout << left << setw(28) << it.name << " -> Avg Waiting = " << fixed << setprecision(2) << it.res.avgWaiting
             << ", Avg Turnaround = " << it.res.avgTurnaround << "\n";
        if(it.res.avgWaiting < best){ best = it.res.avgWaiting; bestName = it.name; }
    }
    cout << "Best (lowest average waiting time) for this workload: " << bestName << "\n";
}

static void sampleData(vector<Process>& procs){
    procs.clear();
    // A small default dataset (can be used to quickly test)
    // P1 arrives 0, burst 7; P2 arr 2, b 4; P3 arr 4, b 1; P4 arr 5, b 4
    procs.push_back({1, "P1", 0, 7, 2});
    procs.push_back({2, "P2", 2, 4, 3});
    procs.push_back({3, "P3", 4, 1, 1});
    procs.push_back({4, "P4", 5, 4, 4});
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<Process> processes; // current dataset
    int quantum = 2;           // default RR quantum

    cout << "CPU Scheduling Algorithm Simulator and Evaluator (C++)\n";
    cout << "Works best in Code::Blocks (Console app).\n";

    // Provide initial hint
    cout << "\nTip: Enter your own processes via option 1. You can also try sample data via option 2.\n";

    while(true){
        cout << "\n================== MAIN MENU ==================\n";
        cout << "1) Enter processes\n";
        cout << "2) Load sample data\n";
        cout << "3) Set Round Robin quantum (current: " << quantum << ")\n";
        cout << "4) Run FCFS\n";
        cout << "5) Run SJF (Non-Preemptive)\n";
        cout << "6) Run Priority (Non-Preemptive)\n";
        cout << "7) Run Round Robin\n";
        cout << "8) Compare all algorithms\n";
        cout << "9) Exit\n";
        int choice = askInt("Select an option (1-9): ", 1, 9);

        if(choice == 1){
            processes = readProcesses();
        } else if(choice == 2){
            sampleData(processes);
            cout << "Sample dataset loaded (4 processes).\n";
        } else if(choice == 3){
            quantum = askInt("Enter time quantum (>0): ", 1, 1e9);
        } else if(choice == 4){
            if(processes.empty()){ cout << "Please enter or load processes first.\n"; continue; }
            runAndShow("FCFS", processes, runFCFS);
        } else if(choice == 5){
            if(processes.empty()){ cout << "Please enter or load processes first.\n"; continue; }
            runAndShow("SJF (Non-Preemptive)", processes, runSJF);
        } else if(choice == 6){
            if(processes.empty()){ cout << "Please enter or load processes first.\n"; continue; }
            runAndShow("Priority (Non-Preemptive)", processes, runPriorityNP);
        } else if(choice == 7){
            if(processes.empty()){ cout << "Please enter or load processes first.\n"; continue; }
            runAndShowRR("Round Robin", processes, quantum);
        } else if(choice == 8){
            if(processes.empty()){ cout << "Please enter or load processes first.\n"; continue; }
            compareAll(processes, quantum);
        } else if(choice == 9){
            cout << "Exiting. Goodbye!\n"; break;
        }
    }
    return 0;
}

