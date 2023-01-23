# include "PTE.h"
# include "Process.h"
# include "Pager.h"
//# include "Pager.cpp"
using namespace std;

Summary summary;
COST cost;

vector<Process> process_list;
vector<frame_t> frame_table;

vector<int> randvals = {};
deque<int> free_list;

Process *current_process = nullptr;
Pager *pager;
char operation = '?';
int target = -404;
int instruction_reader = 0;

char algorithm = 'n';  //n menas not initialized
int frame_number = 0;
int O = 1;
int P = 1;
int F = 1;
int S = 1;
int ofs = 0;

vector<char> instruction = {};
vector<int> instruction_target = {};

void read_rfile(char *name) {
    char buffer[256];
    getcwd(buffer, sizeof(buffer));
    strcat(buffer, "/");
    strcat(buffer, name);
    ifstream f(buffer); //taking file as inputstream
    string str;
    std::getline(f, str);
    while (std::getline(f, str)) {
        randvals.push_back(stoi(str));
    }
}


void choose_pager(char p) {
    switch (p) {
        case 'f':
            pager = new FIFO();
            break;
        case 'r':
            pager = new Random();
            break;
        case 'c':
            pager = new Clock();
            break;
        case 'e':
            pager = new ESC();
            break;
        case 'a':
            pager = new Aging();
            break;
        case 'w':
            pager = new WorkingSet();
            break;

    }
}

void read_arg(int argc, char **argv) {
    int c;
    char *cvalue = NULL;
    string target;
    while ((c = getopt(argc, argv, "f:a:o:")) != -1)
        switch (c) {
            case 'f':
                frame_number = atoi(optarg);
                break;
            case 'a': {
                algorithm = optarg[0];
                choose_pager(algorithm);
                break;
            }
            case 'o': {
                target = optarg;
                if (target.find("O") != string::npos) O = 1;
                if (target.find("P") != string::npos) P = 1;
                if (target.find("F") != string::npos) F = 1;
                if (target.find("S") != string::npos) S = 1;
                break;
            }
            case '?':
                if (optopt == 'c')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                return;
            default:
                abort();
        }
}

void parse_input(char *filename) {
    char buffer[256];
    ifstream myfile(filename);

    for (int i = 0; i < 3; i++) {
        myfile.getline(buffer, 256);
    }

    myfile.getline(buffer, 256);
    int process_num = atoi(buffer);
    for (int i = 0; i < process_num; i++) {

        myfile.getline(buffer, 256);
        myfile.getline(buffer, 256);
        myfile.getline(buffer, 256);
        int vma_num = atoi(buffer);

        Process proc(i);
        process_list.push_back(proc);

        for (int j = 0; j < vma_num; j++) {
            myfile.getline(buffer, 256);
            int a, b, c, d;
            sscanf(buffer, "%d %d %d %d", &a, &b, &c, &d);
            process_list[i].add_vma(a, b, c, d);
        }

        for (int k = 0; k < MAX_VPAGES; k++) {
            pte_t temp = {};
            process_list[i].page_table.push_back(temp);
        }


    }
    myfile.getline(buffer, 256);
    while (1) {
        myfile.getline(buffer, 256);
        if (buffer[0] == '#') break;
        char ins;
        int vpage;
        sscanf(buffer, "%c %d", &ins, &vpage);
        instruction.push_back(ins);
        instruction_target.push_back(vpage);

    }
}


frame_t *allocate_frame_from_free_list() {
    if (free_list.empty()) return nullptr;
    frame_t *frame = &frame_table[free_list.front()];
    free_list.pop_front();
    return frame;

}

void print_frame_table() {
    printf("FT:");
    for (auto frame: frame_table) {
        if (frame.is_using()) printf(" %d:%d", frame.pid, frame.vpage);
        else printf(" *");
    }
    printf("\n");
}

frame_t *get_frame() {
    frame_t *frame = allocate_frame_from_free_list();
    if (frame == nullptr) {
        frame = pager->select_victim_frame();
        if (O) printf(" UNMAP %d:%d\n", frame->pid, frame->vpage);
        process_list[frame->pid].states.unmaps++;

        auto *pte = &process_list[frame->pid].page_table[frame->vpage];
        pte->valid = 0;

        if (pte->modified == 1) {
            if (process_list[frame->pid].is_filemapped(frame->vpage)) {
                if (O) printf(" FOUT\n");
                process_list[frame->pid].states.fouts++;
            } else {
                if (O) printf(" OUT\n");
                pte->pagedout = 1;
                process_list[frame->pid].states.outs++;
            }
        }

    }
    return frame;
}

int get_next_instruction() {
    if (instruction_reader < instruction.size()) {
        operation = instruction[instruction_reader];
        target = instruction_target[instruction_reader];
        instruction_reader++;
        return 1;
    } else return 0;
}

void show_record() {
    if (P) {
        for (auto proc: process_list) {
            proc.print_page_table();
        }
    }
    if (F) {
        print_frame_table();
    }

    for (auto proc: process_list) {
        printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
               proc.pid,
               proc.states.unmaps,
               proc.states.maps,
               proc.states.ins,
               proc.states.outs,
               proc.states.fins,
               proc.states.fouts,
               proc.states.zeros,
               proc.states.segv,
               proc.states.segprot);

    }

    if (S) {
        for (const auto &proc: process_list) {
            summary.cost += proc.states.unmaps * cost.unmaps;
            summary.cost += proc.states.maps * cost.maps;
            summary.cost += proc.states.ins * cost.ins;
            summary.cost += proc.states.outs * cost.outs;
            summary.cost += proc.states.fins * cost.fins;
            summary.cost += proc.states.fouts * cost.fouts;
            summary.cost += proc.states.zeros * cost.zeros;
            summary.cost += proc.states.segv * cost.segv;
            summary.cost += proc.states.segprot * cost.segprot;
        }
        summary.cost += summary.ctx_switches * cost.context_switches;
        summary.cost += summary.process_exits * cost.process_exits;
        summary.cost += (summary.inst_count - summary.ctx_switches - summary.process_exits) * cost.read_write;

        printf("TOTALCOST %lu %lu %lu %llu %lu\n",
               summary.inst_count,
               summary.ctx_switches,
               summary.process_exits,
               summary.cost,
               sizeof(pte_t));
    }

}

void Simulation() {
    for (int i = 0; i < frame_number; ++i) {
        frame_table.emplace_back(i);
        free_list.push_back(i);
    }

    while (get_next_instruction()) {
        printf("%lu: ==> %c %d\n", summary.inst_count++, operation, target);

        if (operation == 'c') {
            current_process = &process_list[target];
            summary.ctx_switches++;
            continue;
        }

        if (operation == 'e') { // process exit
            Process *proc_exiting = &process_list[target];
            pte_t *pte;
            if (O) printf("EXIT current process %d\n", target);
            summary.process_exits++;

            for (int vpg = 0; vpg < MAX_VPAGES; vpg++) {
                pte = &proc_exiting->page_table[vpg];
                pte->pagedout = 0;
                if (pte->valid) {
                    pte->valid = 0;
                    if (O) printf(" UNMAP %d:%d\n", target, vpg);
                    proc_exiting->states.unmaps++;
                    if (pte->modified && proc_exiting->is_filemapped(vpg)) {
                        if (O) printf(" FOUT\n");
                        proc_exiting->states.fouts++;
                    }

                    free_list.push_back(int(pte->frame_number));
                    frame_table[int(pte->frame_number)].pid = -1;
                    frame_table[int(pte->frame_number)].vpage = -1;
                }
            }
            continue;
        }

        pte_t *pte = &current_process->page_table[target];

        if (!pte->valid) {
            if (!current_process->in_vma(target)) {
                printf(" SEGV\n");
                current_process->states.segv++;
                continue;
            }

            frame_t *frame = get_frame();
            if (current_process->is_filemapped(target)) {
                if (O) printf(" FIN\n");
                current_process->states.fins++;
            } else if (pte->pagedout) {
                if (O) printf(" IN\n");
                current_process->states.ins++;
            } else {
                if (O) printf(" ZERO\n");
                current_process->states.zeros++;
            }
            frame->pid = -1;
            frame->vpage = -1;

            pte->modified = 0;
            pte->referenced = 0;
            pte->valid = 1;

            pte->frame_number = unsigned(frame->id);
            frame->pid = current_process->pid;
            frame->vpage = target;
            printf(" MAP %d\n", frame->id);
            current_process->states.maps++;
        }

        pte->referenced = 1;
        if (operation == 'w') {
            if (current_process->is_write_protect(target)) {
                if (O) printf(" SEGPROT\n");
                current_process->states.segprot++;
            } else { pte->modified = 1; }
        }
    }
}


int main(int argc, char **argv) {
//    printf("START");
//    char *data[] = {"./main", "-f32", "-ae", "-oOPFS", "./in11", "./rfile"};
//    argv = data;
//    argc=6;

    read_arg(argc, argv);
    parse_input(argv[argc - 2]);
    read_rfile(argv[argc - 1]);

    Simulation();
    show_record();

    return 0;
}
