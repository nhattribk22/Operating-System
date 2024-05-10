/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef MM_TLB
/*
 * Memory physical based TLB Cache
 * TLB cache module tlb/tlbcache.c
 *
 * TLB cache is physically memory phy
 * supports random access 
 * and runs at high speed
 */

#include "mm.h"
#include <stdlib.h>

#define init_tlbcache(mp,sz,...) init_memphy(mp, sz, (1, ##__VA_ARGS__))
/*define MAX_DATA_PER_PROC for mapping
 *each process gets MAX_DATA_PER_PROC * BYTE data
 */ 
/*256 slots, each slot 8 bytes : 
 * 1 byte for valid
 * 3 bytes for tag (checking TLB MISS/HIT)
 * 4 bytes for page table entry
 */
#define MAX_DATA_PER_PROC 2048

/*
 *  tlb_cache_read read TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_read(struct memphy_struct * mp, int pid, int pgnum, BYTE value)
{
   /* TODO: the identify info is mapped to 
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
   int check_pgnum = pgnum%256;
   int check_addr = 0,return_value = -1;
   for(int i = 0;i<3;i++){
      check_addr= 256*check_addr+mp->storage[check_pgnum*8+1+i];
   }
   //Check_addr is tag, if pgnum matches tag then return page entry (TLB HIT)
   if((pgnum/256)==check_addr&&(int)(mp->storage[check_pgnum*8])!=0x0000){
      int result = 0;
      for(int i = 0;i<4;i++){
         result = 256*result+mp->storage[check_pgnum*8+4+i];
      }
      return result;
   }
   //Else return -1 (TLB MISS)
   else{
      tlb_cache_write(mp,pid,pgnum,value);
      return -1;
   }
}

/*
 *  tlb_cache_write write TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_write(struct memphy_struct *mp, int pid, int pgnum, BYTE value)
{
   /* TODO: the identify info is mapped to 
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
   int write_index = pgnum%256;
   int write_tag = pgnum/256;
   BYTE write_byte;
   //Show that there page table entry exist
   TLBMEMPHY_write(mp,write_index*8,0x0001);
   for(int i = 3;i>0;i++){
      write_byte = write_tag%256;
      TLBMEMPHY_write(mp,write_index*8+i,write_byte);
      write_tag/=256;
   }
   uint32_t data = mp->used_fp_list->owner->pgd[pgnum];
   for(int i = 4;i>0;i++){
      write_byte = data%256;
      TLBMEMPHY_write(mp,write_index*8+3+i,write_byte);
      data/=256;
   }
   return 0;
}

/*
 *  TLBMEMPHY_read natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int TLBMEMPHY_read(struct memphy_struct * mp, int addr, BYTE *value)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   *value = mp->storage[addr];

   return 0;
}


/*
 *  TLBMEMPHY_write natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int TLBMEMPHY_write(struct memphy_struct * mp, int addr, BYTE data)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   mp->storage[addr] = data;

   return 0;
}

/*
 *  TLBMEMPHY_format natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 */


int TLBMEMPHY_dump(struct memphy_struct * mp)
{
   /*TODO dump memphy contnt mp->storage 
    *     for tracing the memory content
    */
   int dump = 0;
   for(int i = 0;i<mp->maxsz;i++){
      dump*=256;
      dump+=atoi(mp->storage[i]);
   }
   return dump;
}


/*
 *  Init TLBMEMPHY struct
 */
int init_tlbmemphy(struct memphy_struct *mp, int max_size)
{
   mp->storage = (BYTE *)malloc(max_size*sizeof(BYTE));
   mp->maxsz = max_size;

   mp->rdmflg = 1;

   return 0;
}

//#endif
