//
//  felix.h
//  ldecod
//
//  Created by Felix Gan on 3/2/24.
//

#ifndef felix_h
#define felix_h

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#define ever ;;

int file_index = -1;

int write_to_file(int size, char* buf) {
    char *filename = "mv_and_texture";
    char *unique_filename = malloc(30*sizeof(char));
    FILE *fptr;
    
    if (file_index == -1) {
        int i = 1;
        for(ever) {
            // check if exists, if so, make new file in sequence
            snprintf(unique_filename, 30, "%s-%i.txt", filename, i);
            
            fptr = fopen(unique_filename, "rb+");
            if (fptr == NULL) break;
            
            i++;
        }
        file_index = i;
    }
    
    snprintf(unique_filename, 30, "%s-%i.txt", filename, file_index);
    fptr = fopen(unique_filename, "a");
    
    if (fptr == NULL) {
        fprintf(stderr, "ERROR opening file %s", filename);
        return 1;
    }
    
    fprintf(fptr, "%s", buf);
    
    fclose(fptr);
    
    // deprecated: character by character printing
//    for (int i = 0; i < size; i++) {
//        char *curr = buf + i;
//        fprintf(fptr, "%s", *curr);
//    }
    
    return 0;

}

char* concat(const char *s1, const char *s2)
{
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    char *result = malloc(len1 + len2 + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1); // +1 to copy the null-terminator
    return result;
}


#endif /* felix_h */
