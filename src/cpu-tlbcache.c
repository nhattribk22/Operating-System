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
int tlb_cache_read(struct memphy_struct * mp, int pid, int pgnum, int frame)
{
   /* TODO: the identify info is mapped to 
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
   int check_pgnum = pgnum%256;
   int check_addr = 0;
   BYTE value;
   for(int i = 0;i<3;i++){
      TLBMEMPHY_read(mp,pid*MAX_DATA_PER_PROC + check_pgnum*8+1+i,&value);
      check_addr= 256*check_addr+(int)(value);
   }
   //Check_addr is tag, if pgnum matches tag then return page entry (TLB HIT)
   TLBMEMPHY_read(mp,pid*MAX_DATA_PER_PROC + check_pgnum*8,&value);
   if(((pgnum/256)==check_addr) && ((BYTE)(value)!=(BYTE)(0x0000))){
      int frame = 0;
      for(int i = 0;i<4;i++){
         TLBMEMPHY_read(mp,pid*MAX_DATA_PER_PROC + check_pgnum*8+4+i,&value);
         frame = 256*frame+(int)(value);
      }
      return frame;
   }
   //Else return -1 (TLB MISS)
   else{
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
int tlb_cache_write(struct memphy_struct *mp, int pid, int pgnum, int frame)
{
   /* TODO: the identify info is mapped to 
    *      cache line by employing:
    *      direct mapped, associated mapping etc.
    */
   int write_index = pgnum%256;
   BYTE write_byte;
   //Show that there page table entry exist (valid byte = 1)
   TLBMEMPHY_write(mp,pid*MAX_DATA_PER_PROC + write_index*8,(BYTE)(0x0001));
   for(int i = 3;i>0;i--){
      write_byte = (BYTE)(pgnum%256);
      TLBMEMPHY_write(mp,pid*MAX_DATA_PER_PROC + write_index*8+i,write_byte);
      pgnum/=256;
   }
   for(int i = 4;i>0;i--){
      write_byte = (BYTE)(frame%256);
      TLBMEMPHY_write(mp,pid*MAX_DATA_PER_PROC + write_index*8+3+i,write_byte);
      frame/=256;
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
   BYTE data;
   printf("TLBMEMPHY_dump : \n");
   for(int i = 0; MAX_DATA_PER_PROC*i<=mp->maxsz;i++){
      for(int j = 0;j<256;j++){
         TLBMEMPHY_read(mp,i*MAX_DATA_PER_PROC+8*j,&data);
         if(data!=(BYTE)0x0000){
            printf("At pid = %d, offset = %d :  ",i,j);
            for(int k = 0;k<8;k++){
               TLBMEMPHY_read(mp,i*MAX_DATA_PER_PROC+8*j+k,&data);
               printf("%X  ",data);
            }
            printf("\n");
         }
      }
   }
   return 0;
}


/*
 *  Init TLBMEMPHY struct
 */
int init_tlbmemphy(struct memphy_struct *mp, int max_size)
{
   mp->storage = (BYTE *)malloc(max_size*sizeof(BYTE));
   mp->maxsz = max_size;

   for(int i = 0;i<max_size;i+=8){
      TLBMEMPHY_write(mp,i,(BYTE)(0x00));
   }
   mp->rdmflg = 1;

   return 0;
}

//#endif
