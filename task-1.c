/*	

	This program implement the First Come First Serve algorithm. 
	It reads the file then stores in the process control block.
	Sort them according to the entry time. 
	Upon a new process arrives, mark it as READY then run it after current process finishes. 			 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct single_line
{
    char *s;
    FILE *fp;
} return_value_rsl;

typedef enum {
    READY,RUNNING,EXIT
} process_state_t;

typedef struct {
   char process_name[11];

   /*Times measured in seconds...*/
   int entry_time;
   int service_time;
   int remaining_time;
   int deadline_time;
   
   //round robin and deadline will use this
   int wait_time;
   int turnaround;

   process_state_t state;

} pcb_t;


//read single line
return_value_rsl read_single_line(FILE*);
//delete a certain character in string
char* delete_char(char*, char);


//sort the process according to entry time
void sort(pcb_t pcbs[], int left, int right)
{
    if (left >= right)
    {
        return;
    }

    int i = left;
    int j = right;
    float key = pcbs[left].entry_time;
    pcb_t key_pcbs = pcbs[left];
    while (left < right)
    {
        while (left < right && key <= pcbs[right].entry_time)
        {
            right--;
        }
        if (key > pcbs[right].entry_time)
        {
            pcbs[left] = pcbs[right];
            left++;
        }
        while (left < right && key >= pcbs[left].entry_time)
        {
            left++;
        }
        if (key < pcbs[left].entry_time)
        {
            pcbs[right] = pcbs[left];
            right--;
        }
    }
    //if left == right, sort two sides seperately
    pcbs[left] = key_pcbs;
    sort(pcbs, i, left - 1);
    sort(pcbs, left + 1, j);
}

int main(void)
{
    //declare pcb data structure which I can assume that the max number of process is 10
    pcb_t pcb[10];

    /*read file "process_data.txt" then put into pcb*/
    FILE *in = fopen("process_data.txt", "r");
    if (in == NULL)
    {
        printf("can not read the file!");
        return -1;
    }

    int count = 0;
    while (1)
    {
        //read line by line
        return_value_rsl val = read_single_line(in);
        in = val.fp;
        //if we get the end of the file, loop stops
        if (feof(in))
        {
            break;
        }

        //delete the "\n" of the line
        char *a_line = delete_char(val.s, '\n');
        if (strcmp(a_line, "") == 0)
        {
            break;
        }

        char delims[] = " ";
        char *result = NULL;
        int j = 0;
        result = strtok(a_line, delims);		//reference: https://www.runoob.com/cprogramming/c-function-strtok.html
        while (result != NULL)
        {
            if (j == 0)
            {
                strcpy(pcb[count].process_name, result);
            }
            else if (j == 1)
            {
                pcb[count].entry_time = atoi(result);		//turn string into integer, ref: https://www.runoob.com/cprogramming/c-function-atoi.html
            }
            else if (j == 2)
            {
                pcb[count].service_time = atoi(result);
            }
            else if (j == 3)
            {
                pcb[count].deadline_time = atoi(result);
            }

            //printf("%s\n", result);
            result = strtok(NULL, delims);
            j++;
        }
        count++;
        //printf("%s", val.s);
    }
    //printf("count is %d\n",count);
    fclose(in);

    sort(pcb, 0, count - 1);
    
	
    //open a file in write mode
    FILE *out = fopen("scheduler_result.txt", "w");
    //case a file cannot be open
	if (out == NULL)
    {
        printf("can not read the file!");
        return -1;
    }
    int current_time = 0;
    int pcb_index = 0;
    //read from pcb, here begins first come first serve
    while (pcb_index < count)
    {
        //case the first process
        if (pcb_index == 0)
        {
        	//there is a Process 1 entrying in time 0
            if (pcb[pcb_index].entry_time == 0)
            {
                printf("Time 0: %s has entered the system.\n", pcb[pcb_index].process_name);
            }
            else
            {
                printf("Time %d: %s has entered the system.\n", pcb[pcb_index].entry_time, pcb[pcb_index].process_name);
            }

            //reset pcb information
            pcb[pcb_index].remaining_time = pcb[pcb_index].service_time;
            pcb[pcb_index].state = READY;
            current_time = pcb[pcb_index].entry_time + current_time;
        }

        printf("Time %d: %s is in the running state.\n", current_time, pcb[pcb_index].process_name);
        pcb[pcb_index].state = RUNNING;
        //output the process name and waiting time
        fprintf(out, "%s %d ", pcb[pcb_index].process_name, current_time - pcb[pcb_index].entry_time);
        //new process comes
        for (int i = 0; i < pcb[pcb_index].service_time; i++)
        {
            sleep(1);
            pcb[pcb_index].remaining_time = pcb[pcb_index].remaining_time - 1;
            current_time = current_time + 1;
            for (int i = pcb_index + 1; i < count; i++)
            {
                /*when current time equals to an entry time, which means here comes a new process,
					print the information that a process entered*/
                if (pcb[i].entry_time == current_time)
                {
                    printf("Time %d: %s has entered the system.\n", current_time, pcb[i].process_name);
                    //reset pcb
                    pcb[i].remaining_time = pcb[i].service_time;
                    pcb[i].state = READY;
                }
            }
        }

        pcb[pcb_index].remaining_time = 0;
        pcb[pcb_index].state = EXIT;
        //see if the process can be done before deadline
        int d_met;
        if ((current_time - pcb[pcb_index].entry_time) <= pcb[pcb_index].deadline_time)
        {
            d_met = 1;
        }
        //case it cannot
        else
        {
            d_met = 0;
        }
        fprintf(out, "%d %d\n", current_time - pcb[pcb_index].entry_time, d_met);
        printf("Time %d: %s has finished the execution.\n", current_time, pcb[pcb_index].process_name);

        pcb_index++;
    }
    fclose(out);
}

/***
Read a line from the file, pass in the file pointer, and return the file pointer. 
	At this time, the file pointer has changed and points to the next line. 
	The next time the function is called, it will read from a new line.
	My assignment is mainly based on this algorithm. 
	An example of this function is as follows:

 FILE *fp = NULL;

    fp = fopen("test.txt", "r");
    if (fp == NULL)
    {
        printf("error read file \n");
        return -1;
    }
    while (1)
    {
        return_value_rsl val = read_single_line(fp);
        fp = val.fp;
        if (feof(fp)){
            break;
        }
        printf("%s", val.s);
    }
    fclose(fp);
***/

//Read a line and put it where buff points
return_value_rsl read_single_line(FILE *fp)
{
    return_value_rsl val;
    val.s = NULL;
    val.fp = NULL;
    char *buff = NULL;
    char *buff_new = NULL;
    int count = 0;
    //while it is not the end of file
    while (!feof(fp))
    {
        count++;
        char c;
        c = getc(fp);
        if (c != '\n')
        {
            buff_new = (char *)realloc(buff, count * sizeof(char));
            if (buff_new != NULL)
            {
                buff = buff_new;
                *(buff + count - 1) = c;
            }
            else
            {
                free(buff);
                printf("error realloc!");

                val.s = "error realloc";
                val.fp = fp;
                return val;
            }
        }
        else
        {
            buff_new = (char *)realloc(buff, count * sizeof(char));
            if (buff_new != NULL)
            {
                buff = buff_new;
                *(buff + count - 1) = c;
                val.s = buff;
                val.fp = fp;
                return val;
            }
            else
            {
                free(buff);
                printf("error realloc!");

                val.s = "error realloc";
                val.fp = fp;
                return val;
            }
        }
    }

    val.s = buff;
    val.fp = fp;
    return val;
}

//delete a certain character, return a new pointer and points to a new string
char* delete_char(char *s, char b)
{
    char *q = NULL;
    char *temp = NULL;
    int count = 0;
    for (; *s != '\0'; s++)
    {
        if (*s != b)
        {
            count++;
            temp = (char *)realloc(q,sizeof(char) * count);
            if (temp != NULL)
            {
                q = temp;
                *(q + count - 1) = *s;
            }
        }
    }
    count++;
    temp = (char *)realloc(q,sizeof(char) * count);
    if (temp != NULL)
    {
        q = temp;
        *(q + count - 1) = *s;
    }
    return q;
}
