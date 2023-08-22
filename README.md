# Systems-Programming

Project Overview: mdadm Linear Device Implementation

Welcome to the mdadm Linear Device Implementation project! This project involves the step-by-step development of a linear device storage system named "mdadm." Each phase builds upon the previous one, progressively enhancing the capabilities of the system.

The project involves implementing a secure and efficient storage system for a cryptocurrency startup using 16 military-grade hard disks configured as JBOD (Just a Bunch of Disks). The system includes mdadm with block caching using the least-recently used (LRU) algorithm to enhance performance, and adds networking support to connect to JBOD servers over the Internet, allowing for on-the-fly switching between JBOD systems to avoid downtime.

Assignment #1 - Initialization and Basic Operations:
In this assignment, you'll initiate the mdadm storage system by unifying multiple disks into a single address space. This involves setting up the necessary data structures, managing the disk array, and handling system mounting and unmounting.

Assignment #2 - Reads and Block Access:
With the basic framework in place, this phase focuses on enabling the mdadm system to read data from disks. You'll implement the mdadm_read function, which allows users to retrieve data from the storage system. The assignment also covers managing different block sizes and disk boundaries during read operations.

Assignment #3 - Writes and Testing:
Building on the previous assignments, this stage introduces write functionality to the mdadm system. You'll create the mdadm_write function, which allows data to be written to the storage system. Rigorous testing will ensure the accuracy of your implementation. Additionally, you'll explore trace replay as a testing technique.

Assignment #4 - Caching for Performance:
The focus of this assignment is on enhancing system performance. You'll implement a block cache using the LRU algorithm, improving latency by storing frequently accessed data in a faster storage medium. The assignment includes creating cache management functions and optimizing access patterns.

Assignment #5 - Networking Support:
In the final phase, you'll add networking capabilities to the mdadm system. This feature enables communication with JBOD systems over a network, utilizing a proprietary protocol. You'll replace local JBOD calls with network calls and implement functions for establishing connections, handling disconnections, and executing JBOD operations over the network.

Each assignment builds on the work completed in the previous stages, gradually shaping the mdadm system into a comprehensive and efficient linear device storage solution. The project covers initialization, read and write functionality, performance optimization through caching, and networking integration. Happy coding!
