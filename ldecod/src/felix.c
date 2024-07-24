
//
//  felix.c
//  ldecod
//
//  Created by Felix Gan on 3/4/24.
//

#include "felix.h"
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
    free(unique_filename);
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

char* concat_and_free(char *dest, const char *src) {
    char *new_str = concat(dest, src); // Use existing concat function
    free(dest); // Free the old destination memory
    return new_str;
}

/* Just a helper function, to keep global frame count.
        Because "(*dec_picture)->frame_num" only counts frames since last IDR
        since it effectively flushes decoder buffer, and starts anew
    By the way, it defines the start of the closed GOP
    "https://ottverse.com/what-are-idr-cra-frames-hevc-differences-uses/#:~:text=IDR%20(Instantaneous%20Decoder%20Refresh)%20and,proper%20video%20decoding%20and%20playback."
 */
int frames_before_last_idr = 0;
int GOP_running_cnt = 0;
Boolean IDR_counted_flag = FALSE;


int count_frame_num(StorablePicture **dec_picture) {
    // first, dynamically update GOP count during first closed GOP group
    int is_idr = (*dec_picture)->idr_flag;
    int cur_frame_num = ((*dec_picture)->frame_poc)/2;
    
    // cnt number of IDRs that has passed
    if (is_idr){
        if (IDR_counted_flag == FALSE) {
            frames_before_last_idr += GOP_running_cnt;
            GOP_running_cnt = 0;
            IDR_counted_flag = TRUE;
        }
    }
    else {
        IDR_counted_flag = FALSE;
        GOP_running_cnt = cur_frame_num >= GOP_running_cnt ? cur_frame_num+1 : GOP_running_cnt;
    }

//    printf("GOP_cnt:%i IDR_cnt: %i cur_frame_num: %i\n", GOP_cnt, IDR_cnt, cur_frame_num);
//        printf("%i ", cur_frame_num);
    
    return frames_before_last_idr + cur_frame_num;
    
}

int extract_mvs_and_write_to_file_std(StorablePicture **dec_picture) {
    
    int mv[3];
    int mm, nn;
    int i=0;
    int j, y, x, mb_height, mb_width, ii=0, jj=0;
    int multiplier;
    
    mb_width = (*dec_picture)->PicWidthInMbs;
    mb_height = (*dec_picture)->PicSizeInMbs/(*dec_picture)->PicWidthInMbs;
    
//    fprintf(stdout, "MB_WIDTH: %d MB_HEIGHT: %d\n", mb_width, mb_height);
    
    multiplier = BLOCK_SIZE;
    
    
    int size_of_buf = 1000;
    char *buf = malloc(size_of_buf*sizeof(char));
    char *final_output = malloc(1*sizeof(char));
    
    
//    write_to_file(size_of_buf, buf);
    
    
    int cnt = 1;
    for(i=0;i<mb_height*4;i++)
    {
        
        mm = i * BLOCK_SIZE;
        char *line_buf = malloc(1*sizeof(char));
        
        for(j=0;j<mb_width*4;j++)
        {
            
//            fprintf(stdout, "cnt: %d\n", cnt++);
            
            nn = j * BLOCK_SIZE;
            
            mv[0] = (*dec_picture)->mv_info[i][j].mv[LIST_0].mv_x ;
            mv[1] = (*dec_picture)->mv_info[i][j].mv[LIST_0].mv_y ;
            mv[2] = (*dec_picture)->mv_info[i][j].ref_idx[LIST_0] ;
            
            // so mv[2], I have no idea what it does, so we're ignoring.
//            fprintf(stdout, "List 0 - x: %i y: %i   |\n", mv[0], mv[1]);
            
            int real_frame_num = count_frame_num(dec_picture);
            
            if (mv[0] != 0 || mv[1] != 0) {
                sprintf(buf, "%i, %i, %i, %i, %i, -1\n",
                        real_frame_num, j*4, i*4, mv[0], mv[1]);
            }
            else {          // if not from previous, then from next
                mv[0] = (*dec_picture)->mv_info[i][j].mv[LIST_1].mv_x ;
                mv[1] = (*dec_picture)->mv_info[i][j].mv[LIST_1].mv_y ;
                mv[2] = (*dec_picture)->mv_info[i][j].ref_idx[LIST_1] ;
                
                
                sprintf(buf, "%i, %i, %i, %i, %i, 1\n",
                        real_frame_num, j*4, i*4, mv[0], mv[1]);
            }
            
            line_buf = concat_and_free(line_buf, buf);
            
//            fprintf(stdout, "List 1 - x: %i y: %i   |\n", mv[0], mv[1]);
            
            
            // this is where we write to file, nicely formatted for later use
            // writing each line of video as one line to file
            
            
        }
        final_output = concat_and_free(final_output, line_buf);
        free(line_buf);
        
    }
    
    final_output = concat_and_free(final_output, "\n\n");
    write_to_file(-1, final_output);
    
    
    free(buf);
    free(final_output);
    
    
    return 0;
    
}






int extract_mvs_and_write_to_file_visual(StorablePicture **dec_picture) {
    
    int mv[3];
    int mm, nn;
    int i=0;
    int j, y, x, mb_height, mb_width, ii=0, jj=0;
    int multiplier;
    
    mb_width = (*dec_picture)->PicWidthInMbs;
    mb_height = (*dec_picture)->PicSizeInMbs/(*dec_picture)->PicWidthInMbs;
    
//    fprintf(stdout, "MB_WIDTH: %d MB_HEIGHT: %d\n", mb_width, mb_height);
    
    multiplier = BLOCK_SIZE;
    
    
    int size_of_buf = 1000;
    char *buf = malloc(size_of_buf*sizeof(char));
    char *final_output = malloc(5*sizeof(char));
    
    sprintf(final_output, "%i\n", (*dec_picture)->frame_poc/2);     // only writing MV from prev reference frame
//    write_to_file(size_of_buf, buf);
    
    
    
    int cnt = 1;
    for(i=0;i<mb_height*4;i++)
    {
        
        mm = i * BLOCK_SIZE;
        char *line_buf = malloc(1*sizeof(char));
        
        for(j=0;j<mb_width*4;j++)
        {
            
//            fprintf(stdout, "cnt: %d\n", cnt++);
            
            nn = j * BLOCK_SIZE;
            
            mv[0] = (*dec_picture)->mv_info[i][j].mv[LIST_0].mv_x ;
            mv[1] = (*dec_picture)->mv_info[i][j].mv[LIST_0].mv_y ;
            mv[2] = (*dec_picture)->mv_info[i][j].ref_idx[LIST_0] ;
            
            // so mv[2], I have no idea what it does, so we're ignoring.
//            fprintf(stdout, "List 0 - x: %i y: %i   |\n", mv[0], mv[1]);
            
            sprintf(buf, "(%i,%i)/", mv[0], mv[1]);     // only writing MV from prev reference frame
            line_buf = concat(line_buf, buf);
            
            mv[0] = (*dec_picture)->mv_info[i][j].mv[LIST_1].mv_x ;
            mv[1] = (*dec_picture)->mv_info[i][j].mv[LIST_1].mv_y ;
            mv[2] = (*dec_picture)->mv_info[i][j].ref_idx[LIST_1] ;
            
            
            sprintf(buf, "(%i,%i) ", mv[0], mv[1]);     // only writing MV from prev reference frame
            line_buf = concat(line_buf, buf);
            
//            fprintf(stdout, "List 1 - x: %i y: %i   |\n", mv[0], mv[1]);
            
            
            // this is where we write to file, nicely formatted for later use
            // writing each line of video as one line to file
            
            
            
        }
        final_output = concat(final_output, line_buf);
        final_output = concat(final_output, "\n");
        
        
    }
    
    final_output = concat(final_output, "\n\n");
    write_to_file(-1, final_output);
    
    
    return 0;
    
}


