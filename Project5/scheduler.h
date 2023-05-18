/*
 * COMP 3500: Project 5 Scheduling
 * Chris CJ Young
 * Apr 23 2023
 *
 * 
 *
 * scheduler.h - header file of scheduler.c
 */

#ifndef _SCHEDULER_H_ 
#define _SCHEDULER_H_

#define MAX_TASK_NUM 32

typedef unsigned int u_int;

typedef struct task_info { 
    u_int pid; // process id
    u_int arrival_time;
    u_int burst_time; // run time
    u_int time_remaining;
    u_int queued; // signals a task is in ready queue
    u_int beginning_time;
    u_int finish_time; // finish time
    u_int null; // signals a task has finished
    u_int Turn_Took; // for round robin
} task_t;

typedef struct stat_info { // stat analysis
    double Avg_Waiting;
    double Avg_TurnAround;
    double Avg_Response;
    double usage_Of_CPU;
} stat_info_t;

// TaskList - the waiting list
// size - the size of waiting list
// Policy - the policy passed in
// TimeQuantum - for round robin

void Task_Manager(task_t TaskList[], int size, char *Policy, u_int TimeQuantum);
// chooses the correct policy function to call

//Task_Array - the waiting list
// count - the size of waiting list

void fcfs_policy(task_t Task_Array[], u_int count);
//implements first come first serve

//Task_Array - the waiting list
// count - the size of waiting list

void srtf_policy(task_t Task_Array[], u_int count);
// This implements the shortest remaining time first

//Task_Array - the waiting list
// count - the size of waiting list
// TimeQuantum - length of equal time slices

void rr_policy(task_t Task_Array[], u_int count, u_int TimeQuantum);
//implements round robin, allows for time discrepencies so that response time is reduced

#endif