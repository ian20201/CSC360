#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include "diskstructs.h"

void getCurrentTime(struct dir_entry_timedate_t *timeb) {
    struct tm *UTCtime;
    time_t now = time(0);
    UTCtime = gmtime(&now);
    timeb->second = (UTCtime->tm_sec);
    timeb->minute = (UTCtime->tm_min);
    timeb->hour = ((UTCtime->tm_hour+16)%24);
    timeb->day = (UTCtime->tm_mday-1);
    timeb->month = (UTCtime->tm_mon+1);
    timeb->year = ((UTCtime->tm_year+1900));
}

void *try_malloc(unsigned long int size){
    void *p = malloc(size); 
    if(p == NULL){
        printf("Error allocating memory");
        exit(1);
    }
    return p;
}