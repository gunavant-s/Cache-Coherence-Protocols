# MESI and MOESI Cache Coherence Protocol Simulation

## Overview
This project implements and simulates two cache coherence protocols: MESI (Modified, Exclusive, Shared, Invalid) and MOESI (Modified, Owned, Exclusive, Shared, Invalid) for multi-processor cache systems.

## Features
- Detailed implementation of MESI and MOESI cache coherence protocols
- Simulation of multi-processor cache interactions
- Performance tracking and analysis
- Latency calculation for various cache operations

## Key Components
1. **Cache Coherence States**
   - MESI: Modified, Exclusive, Shared, Invalid
   - MOESI: Modified, Owned, Exclusive, Shared, Invalid

2. **Performance Metrics Tracked**
   - Read/Write hits and misses
   - Memory transactions
   - Cache-to-cache transfers
   - Invalidations and flushes
   - Total execution time

## Implementation Details
- Processor access simulation
- Bus snooping mechanism
- State transition handling
- Latency calculation for different cache operations

## Compilation Instructions
```bash
g++ -o cache_simulator main.cpp cache.cpp
./cache_simulator [input_trace_file]
