//
// Created by Kessel on 11/25/2022.
//

#ifndef INC_2250_ASSN3_PAGER_H
#define INC_2250_ASSN3_PAGER_H

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
#include <algorithm>

# include "PTE.h"
# include "Process.h"

using namespace std;

extern vector<frame_t> frame_table;
extern vector<int> randvals;
extern int ofs;
extern vector<Process> process_list;
extern Summary summary;

class Pager {
protected:

public:

    Pager();
    virtual frame_t *select_victim_frame() { return nullptr; };

    virtual void get_next(int &num) { num = (num + 1) % int(frame_table.size()); }

};


class FIFO : public Pager {
protected:
    int position = 0;  //or hand

public:
    FIFO() : Pager() {};

    frame_t *select_victim_frame() override {
        frame_t *victim_frame = &frame_table[position];
        get_next(position);
        return victim_frame;
    };
};

class Random : public Pager {

public:
    Random() : Pager() {};

    static int myrandom(int max) {
        int rand = (randvals[ofs] % max);
        ofs++;
        return rand;
    }

    frame_t *select_victim_frame() override {
        frame_t *victim_frame = &frame_table[myrandom(frame_table.size())];
        return victim_frame;
    };
};

class Clock : public Pager {
protected:
    int position = 0;

public:
    Clock() : Pager() {};

    frame_t *select_victim_frame() override {
        frame_t *victim_frame;
        pte_t *pte;
        while (true) {
            victim_frame = &frame_table[position];
            pte = &process_list[victim_frame->pid].page_table[victim_frame->vpage];
            get_next(position);

            if (pte->referenced == 1) pte->referenced = 0;
            else return victim_frame;
        }
    };
};

class ESC : public Pager {
protected:
    int position = 0;
    unsigned long last_reset_time = 0;
    const int RESET_PEROID = 50;

public:
    ESC() : Pager() {};

    frame_t *select_victim_frame() override {
        frame_t *victim_frame;
        pte_t *pte;
        bool do_reset = false;
        int class_index;
        vector<frame_t *> classes(4, nullptr);

        if (summary.inst_count - last_reset_time >= RESET_PEROID) {
            last_reset_time = summary.inst_count;
            do_reset = true;
        }

        int frame_id = position;
        while (true) {
            victim_frame = &frame_table[frame_id];
            pte = &process_list[victim_frame->pid].page_table[victim_frame->vpage];
            class_index = (2 * pte->referenced) + pte->modified;   //=2R+M

            if (do_reset) pte->referenced = 0;
            if (classes[class_index] == nullptr) classes[class_index] = victim_frame;
            get_next(frame_id);

            if (frame_id == position) break;
        }

        for (auto & classe : classes) {
            victim_frame = classe;
            if (classe != nullptr) break;
        }

        position = (victim_frame->id + 1) % int(frame_table.size());
        return victim_frame;

    };
};

class Aging : public Pager {
protected:
    int position = 0;

public:
    Aging() : Pager() {};

    frame_t *select_victim_frame() override {
        frame_t *victim_frame = &frame_table[position];
        frame_t *frame;
        pte_t *pte;

        int frame_id = position;

        while (true) {
            frame = &frame_table.at(frame_id);
            pte = &process_list[frame->pid].page_table[frame->vpage];

            frame->counter >>= 1;
            if (pte->referenced == 1)
                frame->counter = (frame->counter | 0x80000000); //0x80000000 means 100000000000000000000000
            pte->referenced = 0;

            if (victim_frame->counter > frame->counter) victim_frame = frame;
            get_next(frame_id);

            if (frame_id == position) break;
        }

        position = (victim_frame->id + 1) % int(frame_table.size());
        return victim_frame;
    };
};

class WorkingSet : public Pager {
protected:
    int position = 0;
    const int TAU = 49;
public:
    WorkingSet() : Pager() {};

    frame_t *select_victim_frame() override {
        frame_t *victim_frame = &frame_table[position];
        frame_t *frame;
        pte_t *pte;

        int frame_id = position;

        while (true) {
            frame = &frame_table.at(frame_id);
            pte = &process_list[frame->pid].page_table[frame->vpage];

            if (pte->referenced) {
                frame->counter = summary.inst_count;
                pte->referenced = 0;
            } else if (frame->counter < victim_frame->counter) {
                victim_frame = frame;
            }

            if ((summary.inst_count - frame->counter) > TAU) {
                victim_frame = frame;
                break;
            }

            get_next(frame_id);
            if (frame_id == position) break;
        }

        position = (victim_frame->id + 1) % int(frame_table.size());
        return victim_frame;
    };
};


#endif //INC_2250_ASSN3_PAGER_H
