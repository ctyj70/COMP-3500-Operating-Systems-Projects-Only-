/*
 * COMP 3500: Project 5 Scheduler.c
 * Chris CJ Young
 *  Apr 20 2023
 *
 * USed class slides, W3Schools, and the Project 5 description to help develop this
 * 
 * This source code shows how to conduct separate compilation.
 *
 * How to compile using Makefile?
 * $make
 *
 * How to manually compile?
 * $gcc -c open.c
 * $gcc -c read.c
 * $gcc -c print.c
 * $gcc open.o read.o print.o scheduler.c -o scheduler
 *
 * How to run?
 * Case 1: no argument. Sample usage is printed
 * $./scheduler
 * Usage: scheduler <file_name>
 *
 * Case 2: file doesn't exist.
 * $./scheduler file1
 * File "file1" doesn't exist. Please try again...
 *
 * Case 3: Input file
 * $./scheduler task.list
 * data in task.list is printed below...
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scheduler.h"
#include "print.h"
#include "open.h"
#include "read.h"


stat_info_t stats;
u_int idle_time = 0;
u_int quantum;

int main( int argc, char *argv[] )  {
    char *file_name; /* file name from commandline */
    FILE *fp; /* file descriptor */
    task_t Task_Array[MAX_TASK_NUM];

    int error_code;
    u_int i;
    u_int count;
    u_int quantum = -1;

    // Input Validation
    // Checks if correct number of arguments are inputted
    if (argc <= 2) {

        printf("Usage: scheduler task_list_file [FCFS|RR|SRTF] [time_quantum]");
        return EXIT_FAILURE;
    }
    //Makes sure that RR has a time quantum argument
    if (strcmp(argv[2], "RR") == 0) {
        if (argc == 4) {
            printf("time_quantum is currently set to %s\n", argv[3]);
            quantum = atoi(argv[3]);
        }
        else {
            printf("Please enter time_quantum for the RR policy\n");
            return EXIT_FAILURE;
        }
    }

    error_code = open_file(argv[1], &fp);
    if (error_code == 1)
        return EXIT_FAILURE;

    // prints out tasks that are to be run then waits for user input
    printf("Scheduling policy: %s\n", argv[2]);
    read_file(fp, Task_Array, &count);
    print_task_list(Task_Array, count);

    // Ensures boolean checks in the policy functions succeed without problems
    for (int i = 0; i < count; i++) {
        Task_Array[i].queued = 0;
        Task_Array[i].null = 0;
        Task_Array[i].Turn_Took = 0;
    }
    

    printf("==================================================================\n");

    //Task_Manager chooses which policy type to use, then calls it
    Task_Manager(Task_Array, count, argv[2], quantum);

    fclose(fp);
    return EXIT_SUCCESS;
}

void Task_Manager(task_t task_list[], int size, char *procedure, u_int quantum) {
    if (strcmp(procedure, "FCFS") == 0) {
        fcfs_policy(task_list, size);
    }
    else if (strcmp(procedure, "RR") == 0) {
        rr_policy(task_list, size, quantum);
    }
    else if (strcmp(procedure, "SRTF") == 0) {
        srtf_policy(task_list, size);
    }
    // Input is invalid, print string instructing the user of the choices
    else {
        printf("No valid policy was provided. Choices are FCFS, RR, and SRTF.");
        return;
    }
    // print the data from stats
    printf("============================================================\n");
    printf("Average waiting time:    %.2f\n", stats.Avg_Waiting);
    printf("Average response time:   %.2f\n", stats.Avg_Response);
    printf("Average turnaround time: %.2f\n", stats.Avg_TurnAround);
    printf("Average CPU usage:       %.2f%%\n", stats.usage_Of_CPU);
    printf("============================================================\n");
}



void fcfs_policy(task_t Task_Array[], u_int count) {
    u_int Avg_Response = 0;
    u_int usage_Of_CPU = 0;
    u_int Current_Time = 0;
    u_int Avg_Waiting = 0;
    u_int Avg_TurnAround = 0;


    u_int Ready_Size = 0;
    u_int Finish_Size = 0;
    task_t Ready_Array[MAX_TASK_NUM];
    task_t Finish_Array[MAX_TASK_NUM];

    u_int index = -1;

    u_int Resume = 1;
    
    while (Resume == 1) {
        // Searches for task that have "arrived"
        for (int i = 0; i < count; i++) {
            if (Task_Array[i].queued == 0 && Task_Array[i].arrival_time <= Current_Time) {
                Task_Array[i].time_remaining = Task_Array[i].burst_time;
                Ready_Array[Ready_Size] = Task_Array[i];
                Ready_Size++;

                // setting queued reports the task has arrived
                Task_Array[i].queued = 1;
            }
        }
        
        // FCFS chooses the first task in the list 
        for (int i = 0; i < Ready_Size; i++) {

            // Checks if task hasnt finished. 
            if (Ready_Array[i].null == 0) {
                index = i;
                break;
            }
        }

        // If there are no tasks waiting then the cpu remains idle
        if (index == -1 || Ready_Array[index].null == 1) {
            Current_Time++;
            idle_time++;
            continue;
        } 

        // Checks that the task has not started, sets its start time if it hasnt
        if (Ready_Array[index].time_remaining == Ready_Array[index].burst_time) {
            Ready_Array[index].beginning_time = Current_Time;
        }

        // Loop runs the task until finished
        while (Ready_Array[index].time_remaining != 0) {
            printf("<time %u> process %u is running ...\n", Current_Time, Ready_Array[index].pid);
            Ready_Array[index].time_remaining--;
            Current_Time++;
            if (Ready_Array[index].time_remaining == 0) {
                printf("<time %u> process %u is finished ...\n", Current_Time, Ready_Array[index].pid);
            
                Ready_Array[index].finish_time = Current_Time;
                Finish_Array[Finish_Size] = Ready_Array[index];
                Finish_Size++;
                Ready_Array[index].null = 1;
                break;
            }
        }
        // count if there is anything in the waiting/ready queue
        u_int placeholder = 0;
        for (int i = 0; i < count; i++) {
            if (Ready_Array[i].null == 0 || Task_Array[i].queued == 0) {
                placeholder++;
            }
        }
        // if placeholder is zero then both queues are empty and the program ends
        if (placeholder == 0) {
            Resume = 0;
        }
    } 
    // Calculating totals for stats which will be divided by count to get final average
    for (int i = 0; i < count; i++) {
        double response = Ready_Array[i].beginning_time - Ready_Array[i].arrival_time;
        double waiting_time = Ready_Array[i].finish_time - Ready_Array[i].arrival_time - Ready_Array[i].burst_time;
        double trnd = Ready_Array[i].finish_time - Ready_Array[i].arrival_time;
        stats.Avg_Response += response;
        stats.Avg_Waiting += waiting_time;
        stats.Avg_TurnAround += trnd;
    }
    // Stats are saved to be printed after execution of method

    double inverse_usage = (double)(idle_time/Current_Time) * 100;
    stats.usage_Of_CPU = 100-inverse_usage;
    stats.Avg_Response /= count;
    stats.Avg_Waiting /= count;
    stats.Avg_TurnAround /= count;
}

void srtf_policy(task_t Task_Array[], u_int count) {
    u_int Avg_Response = 0;
    u_int usage_Of_CPU = 0;
    u_int Current_Time = 0;
    u_int Avg_Waiting = 0;
    u_int Avg_TurnAround = 0;

    u_int Ready_Size = 0;
    u_int Finish_Size = 0;
    task_t Ready_Array[MAX_TASK_NUM];
    task_t Finish_Array[MAX_TASK_NUM];

    u_int index = -1;

    u_int Resume = 1;
    
    while (Resume == 1) {

        // Gets the newly arrived tasks and adds them to queue
        for (int i = 0; i < count; i++) {
            if (Task_Array[i].queued == 0 && Task_Array[i].arrival_time <= Current_Time) {
                Task_Array[i].time_remaining = Task_Array[i].burst_time;
                Ready_Array[Ready_Size] = Task_Array[i];
                Ready_Size++;
                Task_Array[i].queued = 1;
            }
        }

        // Initializes the shortest remaining to UINT_MAX value
        u_int ShortestRemaining = 4294967295;	

        // This variable keeps track of the stortest remaining task index
        u_int ShortestRemainingIndex = -1;

        // Finds the shortest remaining task index
        for (int i = 0; i < Ready_Size; i++) {
            if (Ready_Array[i].null == 0 && Ready_Array[i].time_remaining < ShortestRemaining) {
                ShortestRemainingIndex = i;
                ShortestRemaining = Ready_Array[i].time_remaining;
            } 
            
            if (ShortestRemainingIndex == -1) {
                break;
            }
            else {
                index = ShortestRemainingIndex;
            }
        }

        // Checks if the cpu is idle and it adds to idle_time if idle
        if (index == -1 || Ready_Array[index].null == 1) {
            Current_Time++;
            idle_time++;
            continue;
        }

        // Sets the start time of currently running task
        if (Ready_Array[index].time_remaining == Ready_Array[index].burst_time) {
            Ready_Array[index].beginning_time = Current_Time;
        }

        //  runs task until finished
        while (Ready_Array[index].time_remaining != 0) {
            printf("<time %u> process %u is running ...\n", Current_Time, Ready_Array[index].pid);
            Ready_Array[index].time_remaining--;
            Current_Time++;
            if (Ready_Array[index].time_remaining == 0) {
                printf("<time %u> process %u is finished ...\n", Current_Time, Ready_Array[index].pid);
                Ready_Array[index].finish_time = Current_Time;
                Finish_Array[Finish_Size] = Ready_Array[index];
                Finish_Size++;
                // Sets null so the task is finished
                Ready_Array[index].null = 1;
                break;
            }
        }

        u_int placeholder = 0;

        // Checks if there are still tasks ready or waiting
        for (int i = 0; i < count; i++) {
            if (Ready_Array[i].null == 0 || Task_Array[i].queued == 0) {
                placeholder++;
            }
        }

        // Ends execution if no tasks are ready or waiting
        if (placeholder == 0) {
            Resume = 0;
        }
    }

    // Calculates the totals for the stats
    for (int i = 0; i < count; i++) {
        double response = Ready_Array[i].beginning_time - Ready_Array[i].arrival_time;
        double waiting_time = Ready_Array[i].finish_time - Ready_Array[i].arrival_time - Ready_Array[i].burst_time;
        double trnd = Ready_Array[i].finish_time - Ready_Array[i].arrival_time;
        stats.Avg_Response += response;
        stats.Avg_Waiting += waiting_time;
        stats.Avg_TurnAround += trnd;
    }
    // Calculates the percent of time CPU was being used
    double divide = (double)(idle_time/Current_Time) * 100;
    stats.usage_Of_CPU = 100-divide;

    // Calculates the averages of all stats
    stats.Avg_Response /= count;
    stats.Avg_Waiting /= count;
    stats.Avg_TurnAround /= count;
}

void rr_policy(task_t Task_Array[], u_int count, u_int quantum) {
    u_int Avg_Response = 0;
    u_int usage_Of_CPU = 0;
    u_int Current_Time = 0;
    u_int Avg_Waiting = 0;
    u_int Avg_TurnAround = 0;

    u_int Ready_Size = 0;
    u_int Finish_Size = 0;
    task_t Ready_Array[MAX_TASK_NUM];
    task_t Finish_Array[MAX_TASK_NUM];

    u_int index = -1;

    u_int Resume = 1;

    // tracks the quantum
    u_int time_since_switch = 0;

    while (Resume == 1) {

        // fetches newly arriving tasks
        for (int i = 0; i < count; i++) {
            if (Task_Array[i].queued == 0 && Task_Array[i].arrival_time <= Current_Time) {
                Task_Array[i].time_remaining = Task_Array[i].burst_time;
                Ready_Array[Ready_Size] = Task_Array[i];
                Ready_Size++;
                Task_Array[i].queued = 1;
            }
        }

        // Verifies if the tasks have been done this iteration, sets 0 to Turn_Took if it did
        if (index >= 0 && index == (Ready_Size - 1)) {
            for (int i = 0; i < Ready_Size; i++) {
                Ready_Array[i].Turn_Took = 0;
            }
        }

        // Fetches the first task where Turn_Took is 0 and hasnt finished
        for (int i = 0; i < Ready_Size; i++) {
            if (i != index && Ready_Array[i].null == 0 && Ready_Array[i].Turn_Took == 0) {
                index = i;
                Ready_Array[i].Turn_Took = 1;
                break;
            }
        }

        // Checks if there are no tasks running adds idle time to the CPU if so
        if (index == -1 || Ready_Array[index].null == 1) {
            Current_Time++;
            idle_time++;
            continue;
        }

        // Sets its start time to Current_Time if process hasnt started
        if (Ready_Array[index].time_remaining == Ready_Array[index].burst_time) {
            Ready_Array[index].beginning_time = Current_Time;
        }

        // Runs the task for the remainder of the time quantum
        while (Ready_Array[index].time_remaining != 0) {
            printf("<time %u> process %u is running ...\n", Current_Time, Ready_Array[index].pid);
            Ready_Array[index].time_remaining--;
            Current_Time++;
            time_since_switch++;

            if (Ready_Array[index].time_remaining == 0) {
                printf("<time %u> process %u is finished ...\n", Current_Time, Ready_Array[index].pid);
                Ready_Array[index].finish_time = Current_Time;
                Finish_Array[Finish_Size] = Ready_Array[index];
                Finish_Size++;
                Ready_Array[index].null = 1;

                // Resets the quantum when the task has finished
                time_since_switch = 0;
                break;
            }

            //  task is changed if time_since_switch equals quantum
            if (time_since_switch == quantum) {
                time_since_switch = 0;
                break;
            }
        }

        // Checks if there are any waiting orready tasks
        u_int placeholder = 0;
        for (int i = 0; i < count; i++) {
            if (Ready_Array[i].null == 0 || Task_Array[i].queued == 0) {
                placeholder++;
            }
        }

        // If there is nothing waiting/ready then program finishes the execution
        if (placeholder == 0) {
            Resume = 0;
        }
    }

    // Calculates the totals for the stats
    for (int i = 0; i < count; i++) {
        double response = Ready_Array[i].beginning_time - Ready_Array[i].arrival_time;
        double waiting_time = Ready_Array[i].finish_time - Ready_Array[i].arrival_time - Ready_Array[i].burst_time;
        double trnd = Ready_Array[i].finish_time - Ready_Array[i].arrival_time;
        stats.Avg_Response += response;
        stats.Avg_Waiting += waiting_time;
        stats.Avg_TurnAround += trnd;
    }
    // Calculates the total cpu usage
    double divide = (double)(idle_time/Current_Time) * 100;
    stats.usage_Of_CPU = 100-divide;
    // This divides the stats by count to get the averages
    stats.Avg_Response /= count;
    stats.Avg_Waiting /= count;
    stats.Avg_TurnAround /= count;
}