#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "cache.h"
using namespace std;

Cache::Cache(int s,int a,int b )
{
   
   ulong i, j;

 lineSize = (ulong)(b);
        sets = 1;               // Only one set for fully associative
        assoc = 10000;        // Large associativity to simulate "infinite" cache
        numLines = assoc;       // The number of lines is equal to associativity
  
   
 
cache = new cacheLine*[sets];
    for (i = 0; i < sets; i++) {
        cache[i] = new cacheLine[assoc];
        for (j = 0; j < assoc; j++) {
            cache[i][j].invalidate();
        }
    }    
   
}

// rw indicates write/read
// if copy = 1 then there is copy in another processor's cache
// cache points to cache class in cache.h
// processor 1 to 4
// ulong state = cache[input_processor]->findLine(addr)->getFlags();

void Cache::MESI_Processor_Access(ulong addr, uchar rw, int copy, Cache **cache, int processor, int num_processors) {
    // Total_execution_time++;
    cacheLine *current_line = cache[processor]->findLine(addr);
    int busrd = 0;
    int busreadx = 0;
    int busupgr = 0;
    bool hit = current_line != NULL ? true : false;
    if (rw == 'r') {
        cache[processor]->reads++;
    } 
    else if (rw == 'w') {
        cache[processor]->writes++;
    }

    if (hit) { // Hit
        updateLRU(current_line);
        ulong current_state = current_line->getFlags();
        // Total_execution_time++;

        if (rw == 'r'){ //PrWr
            // do nothing?
            cache[processor]->Readhits++;
            Total_execution_time+=read_hit_latency;
            if(current_state == INVALID){
                if (copy == 1) {
                //c_to_c_trans++;
                    current_line->setFlags(Shared);
                }
                else {
                    current_line->setFlags(Exclusive);
                }
            }
            
        }
        else if (rw == 'w') { // Write hit
            Total_execution_time+=write_hit_latency;
            cache[processor]->Writehits++;

            if(current_state == INVALID){
                busreadx = 1;
            }
            else if (current_state == Shared) {
                busupgr = 1;
            }
            current_line->setFlags(Modified);
        }
    } 
    else { // Miss
        cacheLine *new_line = cache[processor]->fillLine(addr);
        updateLRU(new_line);
        
        if (rw == 'r') {
            cache[processor]->readMisses++;
            busrd = 1; //anway busrd
            if (copy == 1) {
                //c_to_c_trans++;
                Total_execution_time+= flush_transfer;
                new_line->setFlags(Shared);
            } 
            else {
                cache[processor]->mem_trans++; //3449
                new_line->setFlags(Exclusive);
                Total_execution_time+=memory_latency;
            }
        } 
        else if (rw == 'w') {
            cache[processor]->writeMisses++;
            if (copy == 1) {
                c_to_c_trans++;
                Total_execution_time+= flush_transfer;
            } 
            else { // from mem
                cache[processor]->mem_trans++;  // 76
                Total_execution_time+= memory_latency;
            }
            busreadx = 1;
            new_line->setFlags(Modified);
        }
    }

    for (int i = 0; i < num_processors; i++)
	{
		if (i != processor)
		{
		cache[i]->MESI_Bus_Snoop(addr,i,busrd,busreadx,busupgr);
		}
	}

}

void Cache::MESI_Bus_Snoop(ulong addr, int i, int busread, int busreadx, int busupgrade) {
    cacheLine *current_line = findLine(addr);
    if (current_line != NULL) {
        ulong current_state = current_line->getFlags();
        if (busread == 1) {
            if (current_state == Modified || current_state == Shared || current_state == Exclusive) {
                current_line->setFlags(Shared);
                flushes++;
            } 
            // else if (current_state == Exclusive) {
            //     current_line->setFlags(Shared);
            //     flushes++;
            // }
        }

        if (busreadx == 1) {
            if(current_state == Modified){
                mem_trans++;  // 45
                flushes++;
            }
            else if(current_state == Exclusive || current_state == Shared){
                flushes++;
            }
            current_line->invalidate();
            invalidations++;
            // }
        }
        if (busupgrade == 1) {
            if (current_state == Shared) {
                current_line->invalidate();
                invalidations++;
            }
        }
    }
}

void Cache::MOESI_Processor_Access(ulong addr,uchar rw, int copy, Cache **cache, int processor, int num_processors )
{
    // Total_execution_time++; //10000
    cacheLine *current_line = cache[processor]->findLine(addr);
    int busrd = 0;
    int busreadx = 0;
    int busupgr = 0;
    bool hit = current_line != NULL ? true : false;

    if(rw == 'r'){
        cache[processor]->reads++;
    }
    else if(rw == 'w'){
        cache[processor]->writes++;
    }
        
    if(hit){ //Hit
        updateLRU(current_line);
        ulong current_state = current_line->getFlags();
        // Total_execution_time++;
        
        if(rw == 'r'){ //rd hit
            Total_execution_time+=read_hit_latency;
            cache[processor]->Readhits++;
            if(current_state == INVALID){
                if(copy == 1){
                    current_line->setFlags(Shared);
                }
                else{
                    current_line->setFlags(Exclusive);
                }
            }
        }
        else if (rw == 'w'){ // write hit
            Total_execution_time+=write_hit_latency;
            cache[processor]->Writehits++;

            if(current_state == INVALID){
                busreadx = 1;
            }
            else if(current_state == Shared || current_state == Owner){
                busupgr = 1;
            }    
            current_line->setFlags(Modified);
        }
    }
    else{ //miss
        cacheLine *new_line =  cache[processor]->fillLine(addr);
        updateLRU(new_line);
        
        if(rw == 'r'){
            cache[processor]->readMisses++;
            busrd = 1;
            if(copy == 1){
                // c_to_c_trans++;
                Total_execution_time+= flush_transfer;
                new_line->setFlags(Shared);
            }
            else{
                cache[processor]->mem_trans++; 
                Total_execution_time+=memory_latency;               
                new_line->setFlags(Exclusive);
            }
        }
        else{ // write miss
            cache[processor]->writeMisses++;
            if (copy == 1) {
                c_to_c_trans++;
                Total_execution_time+= flush_transfer;
            } 
            else { // from mem
                cache[processor]->mem_trans++;  // 76
                Total_execution_time+= memory_latency;
            }
            busreadx = 1;
            new_line->setFlags(Modified);
        }
    }

    for (int i = 0; i < num_processors; i++)
	{
		if (i != processor)
		{
		cache[i]->MOESI_Bus_Snoop(addr,i,busrd,busreadx,busupgr);
		}
	}
}

void Cache::MOESI_Bus_Snoop(ulong addr ,int processor, int busread,int busreadx, int busupgrade ){
	cacheLine *current_line = findLine(addr);
    if (current_line != NULL) {
        ulong current_state = current_line->getFlags();
        if (busread == 1) {
            if (current_state == Modified || current_state == Owner) {
                current_line->setFlags(Owner);
                // Total_execution_time+=flush_transfer;
                flushes++;
            } 
            else if (current_state == Exclusive) {
                current_line->setFlags(Shared);
            }
        }

        else if (busreadx == 1) {
            if(current_state == Modified || current_state == Owner){
                // mem_trans++;  // 45 
                // Total_execution_time+=flush_transfer;
                flushes++;
            }
            current_line->invalidate();
            invalidations++;
        }
        else if (busupgrade == 1) {
            if (current_state == Shared || current_state == Owner) {
                current_line->invalidate();
                invalidations++;
            }
        }
    }
}

/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
	if(cache[i][j].isValid())
	        if(cache[i][j].getTag() == tag)
		{
		     pos = j; break;  
		}
   if(pos == assoc)
	return NULL;
   else
	return &(cache[i][pos]); 
}

 
/*Most of these functions are redundant so you can use/change it if you want to*/

/*upgrade LRU line to be MRU line*/
// can be ignored as the cache would only have cold misses
void Cache::updateLRU(cacheLine *line)
{
  line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) 
	  return &(cache[i][j]);     
   }   
   for(j=0;j<assoc;j++)
   {
	 if(cache[i][j].getSeq() <= min) { victim = j; min = cache[i][j].getSeq();}
   } 
   assert(victim != assoc);
   std::cout << "victim" << victim << std::endl;
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);

   assert(victim != 0);
   if ((victim->getFlags() == Modified))
   {
	   writeBack(addr);
   }
   victim->setFlags(Shared);	
   tag = calcTag(addr);   
   victim->setTag(tag);
       
 

   return victim;
}

void Cache::printStats()
{ 
	//printf("===== Simulation results      =====\n");
	float miss_rate = (float)(getRM() + getWM()) * 100 / (getReads() + getWrites());
	
printf("01. number of reads:                                 %10lu\n", getReads());
printf("02. number of read misses:                           %10lu\n", getRM());
printf("03. number of writes:                                %10lu\n", getWrites());
printf("04. number of write misses:                          %10lu\n", getWM());
printf("05. number of write hits:                            %10lu\n", getWH());
printf("06. number of read hits:                             %10lu\n", getRH()); // Changed from getRM() to getRH()
printf("07. total miss rate:                                 %10.2f%%\n", miss_rate);
printf("08. number of memory accesses (exclude writebacks):  %10lu\n", mem_trans);
printf("09. number of invalidations:                         %10lu\n", Invalidations());
printf("10. number of flushes:                               %10lu\n", Flushes());

	
}

void Cache::printCacheState(ulong state) {
    switch (state) {
        case INVALID:
            std::cout << "I";
            break;
        case Shared:
            std::cout << "S";
            break;
        case Modified:
            std::cout << "M";
            break;
        case Exclusive:
            std::cout << "E";
            break;
        default:
            std::cout << "-";
            break;
    }
}
