#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "mdadm.h"
#include "jbod.h"

uint32_t encode_operation(jbod_cmd_t cmd, int disk_num, int block_num)
{

  uint32_t op = cmd << 26 | disk_num << 22 | block_num;
  return op;
}

// Translates a linear address into its corresponding disk number, block number, and offset
void translate_address(uint32_t linear_address, int *disk_num, int *block_num, int *offset) {
  // Calculate the disk number
  *disk_num = linear_address / JBOD_DISK_SIZE;
  
  // Calculate block number and offset within the block
  *block_num = (linear_address % JBOD_DISK_SIZE) / JBOD_BLOCK_SIZE;
  *offset = (linear_address % JBOD_DISK_SIZE) % JBOD_BLOCK_SIZE;
}




// Seeks to the specified block on the specified disk
int seek(int disk_num, int block_num) {
  // Encode operations to seek to disk and block
  uint32_t op_seek_to_disk = encode_operation(JBOD_SEEK_TO_DISK, disk_num, 0);
  uint32_t op_seek_to_block = encode_operation(JBOD_SEEK_TO_BLOCK, 0, block_num);

  // Send the seek-to-disk operation
  jbod_client_operation(op_seek_to_disk, NULL);

  // Send the seek-to-block operation
  jbod_client_operation(op_seek_to_block, NULL);

  // Return success
  return 1;
}


// Initialize boolean variable to check if device is already mounted
bool is_mounted = false;

int mdadm_mount(void) {
 if (is_mounted) {
    return -1;
	}


  uint32_t op = encode_operation(JBOD_MOUNT, 0, 0);
  int result = jbod_client_operation(op, NULL);
  if (result == 0) {
    is_mounted = true; // update flag
    return 1; // success
  } else {
    return -1; // error
  }
}

int mdadm_unmount(void) {
if (!is_mounted) {
  return -1;
}
  uint32_t last_op = encode_operation(JBOD_UNMOUNT, 0, 0);
  int result = jbod_client_operation(last_op, NULL);
  if (result == 0) {
    is_mounted = false; // update boolean
    return 1; // success
  } else {
    return -1; // error
  }
}

int mdadm_read(uint32_t addr, uint32_t len, uint8_t *buf) {
  // Check for invalid state
  if (!is_mounted || len > 1024 || (len && buf == NULL) || addr + len >= (JBOD_NUM_DISKS * JBOD_DISK_SIZE)) {
  return -1;
}
	
  // Read data from disk
  int total_read = 0;
  int curr_addr = addr;
  
  
  uint8_t buf1[256];
  
  
  while (total_read < len) {
    // Translate the current address into disk number, block number, and offset
    int disk_num, block_num, offset;
    translate_address(curr_addr, &disk_num, &block_num, &offset);
    
    // Seek to the current block
    seek(disk_num, block_num);
    
    
 
    if (cache_enabled()==true)
        {

          if ( cache_lookup(disk_num, block_num, buf1) == 1)
	    {
	      memcpy(buf,buf1,256-offset);
	      return 1;
	    }
	} 

    
    // Read data from the current block
    uint8_t buf1[JBOD_BLOCK_SIZE];
    uint32_t op = encode_operation(JBOD_READ_BLOCK, 0, 0);
    jbod_client_operation(op, buf1);
    
    cache_insert(disk_num, block_num, buf1);
    
    // Determine how much data to read from the block
    int remaining = len - total_read;
    int data_read = remaining < JBOD_BLOCK_SIZE - offset ? remaining : JBOD_BLOCK_SIZE - offset;
    
    // Copy data to output buffer
    memcpy(buf + total_read, buf1 + offset, data_read);
    
    // Update counters and addresses for the next iteration
    total_read += data_read;
    curr_addr += data_read;
    offset = 0; // Reset offset after the first block
  }
  
  // Return the number of bytes read
  return total_read;
}


int mdadm_write(uint32_t addr, uint32_t len, const uint8_t *buf) {
    // Check if the disk is mounted and the buffer length is within the limit
    // and the buffer is not null and the total address is within
    // the bounds of available disks and disk space.
    if (!is_mounted || len > 1024 || (len && buf == NULL) || (addr+len) > (JBOD_NUM_DISKS * JBOD_DISK_SIZE)) {
        return -1;
    }

    int disk_num, block_num, offset;
    int total_read = len;
 
    int curr_addr = addr;
    int sumDR = 0;
    
    uint8_t buf1[256];
    int data_read = 0;
    int i=0; // counter
while (curr_addr < addr + len) { // loop until the address matches

    translate_address(curr_addr, &disk_num, &block_num, &offset); // finds the disk num, block num and offset num
    seek(disk_num, block_num); 
    uint32_t op = encode_operation(JBOD_READ_BLOCK, 0, 0); 
    jbod_client_operation(op, buf1); 

    if (i == 0) 
    {
    	memcpy(buf1 + offset, buf + sumDR, ((JBOD_BLOCK_SIZE - offset) < len ? (JBOD_BLOCK_SIZE - offset) : len));
        
        seek(disk_num, block_num);
        uint32_t ops = encode_operation(JBOD_WRITE_BLOCK, 0, 0); // writes on disk
        jbod_client_operation(ops, buf1);
        
        data_read = ((JBOD_BLOCK_SIZE - offset) < len ? (JBOD_BLOCK_SIZE - offset) : len); 

        i =i + 1; 
    }
    else if (total_read < JBOD_BLOCK_SIZE) // last block
    {
        memcpy(buf1, buf + sumDR, total_read);
        seek(disk_num, block_num);
        uint32_t ops = encode_operation(JBOD_WRITE_BLOCK, 0, 0);
        jbod_client_operation(ops, buf1);

        data_read = total_read;
    }
    else
    {
        memcpy(buf1, buf + sumDR, JBOD_BLOCK_SIZE); 
        seek(disk_num, block_num);
        uint32_t ops = encode_operation(JBOD_WRITE_BLOCK, 0, 0);
        jbod_client_operation(ops, buf1);

        data_read = JBOD_BLOCK_SIZE;
    }

    sumDR = sumDR + data_read; // total amount of data read
    total_read = total_read - data_read; 
    curr_addr = curr_addr + data_read; 
}

return len;
}

