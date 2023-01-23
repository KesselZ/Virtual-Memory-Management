#ifndef INC_2250_ASSN3_PROCESS_H
#define INC_2250_ASSN3_PROCESS_H

#define MAX_VPAGES 64

#include <vector>
# include "PTE.h"
using namespace std;

class Process{
public:
    int pid;
//    pte_t page_table[MAX_VPAGES];
    vector<pte_t> page_table;
    vector<VMA> vmas;
    Process(int );

    struct ProcessState {
        unsigned long unmaps=0;
        unsigned long maps=0;
        unsigned long ins=0;
        unsigned long outs=0;
        unsigned long fins=0;
        unsigned long fouts=0;
        unsigned long zeros=0;
        unsigned long segv=0;
        unsigned long segprot=0;
    } states;

    void add_vma(int starting_vpage,int ending_vpage,int write_protected,int file_mapped){
        VMA temp={starting_vpage,ending_vpage,write_protected,file_mapped};
        vmas.push_back(temp);
    };

    bool in_vma(int vpage){
        bool inside=false;
        for(auto & vma : vmas){
            if(vpage>=vma.starting_vpage&&vpage<=vma.ending_vpage) inside= true;
        }
        return inside;
    };

    bool is_filemapped(int vpage){
        for(auto & vma : vmas){
            if(vpage>=vma.starting_vpage && vpage<=vma.ending_vpage) {
                page_table[vpage].filemapped=vma.file_mapped;
                return page_table[vpage].filemapped;
            }
        }
    }

    bool is_write_protect(int vpage){
        for(auto & vma : vmas){
            if(vpage>=vma.starting_vpage && vpage<=vma.ending_vpage) {
                page_table[vpage].write_protect=vma.write_protected;
                return page_table[vpage].write_protect;
            }
        }
    }

    void print_page_table(){
        printf("PT[%d]:", pid);
        for (int i = 0; i < page_table.size(); i++) {
            pte_t &pte = page_table[i];
            if (in_vma(i)) {
                if (pte.valid) {
                    printf(" %d:", i);
                    if(pte.referenced)printf("R");
                    else printf("-");
                    if(pte.modified)printf("M");
                    else printf("-");
                    if(pte.pagedout)printf("S");
                    else printf("-");
                } else {
                    if(pte.pagedout)printf(" #");
                    else printf(" *");
                }
            } else {
                printf(" *");
            }
        }
        printf("\n");
    }
};


#endif //INC_2250_ASSN3_PROCESS_H
