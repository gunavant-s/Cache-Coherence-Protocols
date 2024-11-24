#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "cache.h"
#include <iomanip>
//#include "globals.h"
using namespace std;
unsigned long Total_execution_time = 0;
int main(int argc, char *argv[]) {
    int cache_size = 0;
    int cache_assoc = 0;
    int blk_size = 0;
    int num_processors = 0;
    int protocol = -1;
    char *fname = NULL;
    //unsigned long Total_execution_time = 0; 
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--cache-size") == 0) {
            if (strcmp(argv[i + 1], "infinite") == 0) 
                cache_size = -1;  // Use -1 for "infinite"
            else 
                cache_size = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--assoc") == 0) {
            cache_assoc = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--block-size") == 0) {
            blk_size = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--num-proc") == 0) {
            num_processors = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--protocol") == 0) {
            protocol = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--trace") == 0) {
            fname = argv[i + 1];
            i++;
        }
    }

    // Check if all required arguments were provided
    if (cache_size == 0 || cache_assoc == 0 || blk_size == 0 || num_processors == 0 || protocol == -1 || fname == NULL) {
        cout << "Invalid arguments. Please check the input format." << endl;
        cout << "Usage: ./smp_cache --cache-size <size|infinite> --assoc <assoc> --block-size <size> "
             << "--num-proc <num_processors> --protocol <0|1> --trace <trace_file>" << endl;
        return 1;
    }

    // Print configuration details
    cout << "===== 506 SMP Simulator configuration =====" << endl;
    cout << "L1_SIZE:           " << (cache_size == -1 ? "infinite" : to_string(cache_size)) << endl;
    cout << "L1_BLOCKSIZE:      " << blk_size << endl;
    cout << "NUMBER OF PROCESSORS: " << num_processors << endl;
    cout << "COHERENCE PROTOCOL: " << (protocol == 0 ? "MESI" : "MOESI") << endl;
    cout << "TRACE FILE:        " << fname << endl;

    // Create an array of caches,    length
    Cache **cache = (Cache **)malloc(num_processors * sizeof(Cache *));
    for (int i = 0; i < num_processors; i++) {
        cache[i] = new Cache(cache_size, cache_assoc, blk_size);
    }

    // Open the trace file
    ifstream trace_file(fname);
    if (!trace_file.is_open()) {
        cout << "Trace file problem" << endl;
        return 1;
    }

    string line;
    while (getline(trace_file, line)) {
        istringstream iss(line);
        int input_processor;
        char rw;
        unsigned int addr;

        if (!(iss >> input_processor >> rw >> hex >> addr)) {
            cout << "Error reading trace file format." << endl;
            break;
        }

        // Determine if there is a copy in any other processor's cache
        int copy = 0;
        for (int i = 0; i < num_processors; i++) {
            if (cache[i]->findLine(addr) && (i != input_processor)) {
                copy = 1;
            }
        }

        // Process the request based on the coherence protocol
        if (protocol == 0) {  // MESI protocol
            cache[input_processor]->MESI_Processor_Access(addr, rw, copy, cache, input_processor, num_processors);
              std::cout << std::setw(10) << " " << rw  << ":" << std::hex << addr << "   :P" << (input_processor + 1) << "  ";
              cacheLine *current_line = cache[input_processor]->findLine(addr);
              if(current_line){
                ulong state = current_line->getFlags();
                cache[input_processor]->printCacheState(state);
              }
              for (int i = 0; i < num_processors; i++) {
              if (i != input_processor) {
              cacheLine* line = cache[i]->findLine(addr);
              if (line) {  // If the line exists in the cache
              std::cout << " :P" << (i + 1) << " ";
              cache[i]->printCacheState(line->getFlags());}
              else
              {
                std::cout << " :P" << (i + 1) << " ";
                std::cout << "-";
              }
             }
            }
            std::cout<<std::endl;
            
           }
           
         
         else if (protocol == 1) {  // MOESI protocol
            cache[input_processor]->MOESI_Processor_Access(addr, rw, copy, cache, input_processor, num_processors);
            std::cout << std::setw(10) << " " << rw  << ":" << std::hex << addr << "   :P" << (input_processor + 1) << "  ";
              cacheLine *current_line = cache[input_processor]->findLine(addr);
              if(current_line){
                ulong state = current_line->getFlags();
                cache[input_processor]->printCacheState(state);
              }
              for (int i = 0; i < num_processors; i++) {
              if (i != input_processor) {
              cacheLine* line = cache[i]->findLine(addr);
              if (line) {  // If the line exists in the cache
              std::cout << " :P" << (i + 1) << " ";
              cache[i]->printCacheState(line->getFlags());}
              else
              {
                std::cout << " :P" << (i + 1) << " ";
                std::cout << "-";
              }
             }
            }
            std::cout<<std::endl;
        }
    }

    trace_file.close();
    //printf("Total Execution time: 				                %10lu\n", Total_execution_time);
    printf("Total Execution time:                                 %10lu\n", Total_execution_time);
    // Print out all caches' statistics
    for (int i = 0; i < num_processors; i++) {
        cout << "============ Simulation results (Cache " << i << ") ============" << endl;
        cache[i]->printStats();
    }

    // Free allocated memory
    for (int i = 0; i < num_processors; i++) {
        delete cache[i];
    }
    free(cache);

    return 0;
}
