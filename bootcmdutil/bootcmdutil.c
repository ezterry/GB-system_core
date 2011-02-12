/* system/core/bootcmdutil/bootcmdutil.c
**
** Copyright 2011, ezterry (forum.xda-developers.com)
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

/*
 * Boot image command line utility as initially outlined on XDA:
 * http://forum.xda-developers.com/showpost.php?p=11033890&postcount=1107
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <bootimg.h>

/* Commands */
#define CMD_HELP    0
#define CMD_PRINT   1
#define CMD_RAW     2
#define CMD_APPEND  3
#define CMD_SET     4

void f_cmd_print(char* bootfile){
    FILE* fp = fopen(bootfile,"rb");
    boot_img_hdr header;
    
    if(!fp){
        printf("Error opening %s",bootfile);
        return;
    }
    
    fread(&header, sizeof(header), 1, fp);
    printf("%s's command line: %s\n",bootfile,header.cmdline);
    fclose(fp);
}

int f_cmd_raw(char* bootfile){
    FILE* fp = fopen(bootfile,"rb");
    boot_img_hdr header;
    
    if(!fp){
        return -1;
    }
    
    fread(&header, sizeof(header), 1, fp);
    printf("%s\n",header.cmdline);
    fclose(fp);
    return 0;
}

int f_cmd_append(char* bootfile,char* cmdline){
    FILE* fp = fopen(bootfile,"rb+");
    boot_img_hdr header;
    int sz=BOOT_ARGS_SIZE;
    unsigned char* buffer;
    
    
    if(!fp){
        return -1;
    }
    
    /*read header and prepare to overwrite*/
    fread(&header, sizeof(header), 1, fp);
    fseek(fp,0,SEEK_SET);
    
    buffer = header.cmdline;
    while((buffer[0]) != '\0' && sz > 0){
        buffer=&buffer[1];
        sz--;
    }
    
    if(sz > 0){
        buffer[0]=' ';
        buffer=&buffer[1];
        sz--;
    }
    
    strncpy((char*)buffer,cmdline,(size_t)sz);
    header.cmdline[BOOT_ARGS_SIZE-1]='\0';
    
    printf("New commandline: %s\n",header.cmdline);
    
    fwrite(&header, sizeof(header), 1, fp);
    
    fclose(fp);
    return 0;
}

int f_cmd_set(char* bootfile,char* cmdline){
    FILE* fp = fopen(bootfile,"rb+");
    boot_img_hdr header;
    
    
    if(!fp){
        return -1;
    }
    
    /*read header and prepare to overwrite*/
    fread(&header, sizeof(header), 1, fp);
    fseek(fp,0,SEEK_SET);
    
    memset(header.cmdline,0,BOOT_ARGS_SIZE);
    strncpy((char*)header.cmdline,cmdline,(size_t)BOOT_ARGS_SIZE);
    header.cmdline[BOOT_ARGS_SIZE-1]='\0';
    
    printf("New commandline: %s\n",header.cmdline);
    
    fwrite(&header, sizeof(header), 1, fp);
    
    fclose(fp);
    return 0;
}

int main(int argc, char** argv){
    int runtime_cmd = CMD_HELP;
    
    if(argc > 1){
        if(!strcmp(argv[1],"print")){
            runtime_cmd = CMD_PRINT;
        } 
        else if (!strcmp(argv[1],"raw")){
            runtime_cmd = CMD_RAW;
        }
        else if (!strcmp(argv[1],"append")){
            runtime_cmd = CMD_APPEND;
        }
        else if (!strcmp(argv[1],"set")){
            runtime_cmd = CMD_SET;
        }
        else if (strcmp(argv[1],"--help")){
            /* Anything but "--help" */
            printf("Unknown command: %s\n",argv[1]);
        }
    }
    /* run the command */
    switch(runtime_cmd){
        case CMD_HELP:
            printf("Bootimage Command-line Utility!!!\n\n");
            printf("\nUsage %s\n",argv[0]);
            printf("> %s print <boot.img> \n\t- prints commandline (user friendly)\n",argv[0]);
            printf("> %s raw <boot.img> \n\t- prints commandline no extra text (script friendly)\n",argv[0]);
            printf("> %s append <boot.img> <text>  \n\t- append text to the current command line\n",argv[0]);
            printf("> %s set <boot.img> <text> \n\t- set the command line to text\n",argv[0]);
            break;
        case CMD_PRINT:
            if(argc != 3)
                printf("Usage: %s print <boot.img> \n\t- prints commandline (user friendly)\n",argv[0]);
            else
                f_cmd_print(argv[2]);
            break;
        case CMD_RAW:
            if(argc != 3)
                printf("Usage: %s raw <boot.img> \n\t- prints commandline no extra text (script friendly)\n",argv[0]);
            else
                return(f_cmd_raw(argv[2]));
            break;
        case CMD_APPEND:
            if(argc != 4)
                printf("Usage: %s append <boot.img> <text>  \n\t- append text to the current command line\n",argv[0]);
            else
                return(f_cmd_append(argv[2],argv[3]));
            break;
        case CMD_SET:
            if(argc != 4)
                printf("Usage: %s set <boot.img> <text> \n\t- set the command line to text\n",argv[0]);
            else
                return(f_cmd_set(argv[2],argv[3]));
            break;
    }
    return 0;
}
