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
#include "mbuffer.h"

extern int write_to_file(int size, char* buf);
extern char* concat(const char *s1, const char *s2);
int count_frame_num(StorablePicture **dec_picture);
extern int extract_mvs_and_write_to_file_std(StorablePicture **dec_picture);
extern int extract_mvs_and_write_to_file_visual(StorablePicture **dec_picture);

#endif /* felix_h */
