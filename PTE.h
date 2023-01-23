//
// Created by Kessel on 11/24/2022.
//

#ifndef INC_2250_ASSN3_PTE_H
#define INC_2250_ASSN3_PTE_H
#include <iostream>
#include <typeinfo>
#include <string>
#include <queue>
#include <cstring>
#include<sstream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>

struct pte_t {   //32bits in total
public:
    unsigned pagedout: 1;
    unsigned referenced: 1;
    unsigned modified: 1;
    unsigned write_protect: 1;
    unsigned valid: 1;
    unsigned frame_number: 7;

    unsigned filemapped:1;
    unsigned free_bits: 19;

};

struct VMA {
    int starting_vpage;
    int ending_vpage;
    int write_protected;
    int file_mapped;
};

struct frame_t {
    int id;
    int pid = -1;
    int vpage = -1;
    unsigned int counter = 0;

    explicit frame_t(int i) : id(i) {};
    bool is_using() const { return pid != -1 && vpage != -1; }


};

struct COST {
    const int read_write = 1;
    const int context_switches = 130;
    const int process_exits = 1250;
    const int maps = 300;
    const int unmaps = 400;
    const int ins = 3100;
    const int outs = 2700;
    const int fins = 2800;
    const int fouts = 2400;
    const int zeros = 140;
    const int segv = 340;
    const int segprot = 420;
} ;

struct Summary {
    unsigned long inst_count = 0;
    unsigned long ctx_switches = 0;
    unsigned long process_exits = 0;
    unsigned long long cost = 0;
} ;



#endif //INC_2250_ASSN3_PTE_H
