#include "headers.h"
#include "datastructure.h"
#include "memory.h"
#include "queue.h"
#include "pcb.h"
#include <math.h>

int key_id, key_id2;
int msgq_id, shmid;
// struct msgbuff
// {
//     long mtype;
//     char mtext[70];
// };
void intialize_message_queue();
void intialize_shared_memory();
struct PCB *generate_process();
void round_robin(int Q);
void multilevel(int Q);
void tostring(char str[], int num);
void handler(int signum);
void increment_processes_terminated(int signum);
void forking(struct PCB *process);
void SJF();
void HPF();
int process_count = 0, finished = 0;
int processes_terminated = 0, lastid;
int algorithm_type, waiting_time_utili = 0, time = 0;
float AvgWTA = 0;
float Avg_waiting = 0;
int current_level = 11;
FILE *file;

memory mem;
Queue_final *waiting_queue;
PCB *waiting_process = NULL;
void fork_allocate(PCB *process);
void delocation_forking(PCB *current_process);

int main(int argc, char *argv[])
{
    initialize_memory(&mem, 1024);
    waiting_queue = createQueue_final();
    file = fopen("inputs_logs/memory.log", "w");
    if (file == NULL)
    {
        printf("Error opening file!\n");
    }
    fclose(file);

    // printf("schedular id =%d\n",getpid());
    signal(SIGUSR1, handler);
    signal(SIGUSR2, increment_processes_terminated);
    intialize_message_queue();
    // intialize_shared_memory();
    file = fopen("inputs_logs/scheduler.log", "w");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        return 1; // Exit if file cannot be opened
    }
    initClk();
    algorithm_type = atoi(argv[1]);
    int Quantum = atoi(argv[2]);
    time = 1;
    if (algorithm_type == 3)
    {
        printf("round_robin \n");
        round_robin(Quantum);
    }
    else if (algorithm_type == 4)
    {
        printf("multilevel \n");
        multilevel(Quantum);
    }
    else if (algorithm_type == 1)
    {
        printf("sjf \n");
        SJF();
    }
    else
    {
        printf("hpf \n");
        HPF();
    }

    file = fopen("outs/scheduler.perf", "w");

    // Check if file is successfully opened
    if (file == NULL)
    {
        printf("Error: Could not create file.\n");
    }
    // Write the performance data to the file
    // fprintf(file, "scheduler.perf example\n");
    printf("wait %d\n", waiting_time_utili);
    float clk = getClk();
    fprintf(file, "CPU utilization = %f%%\n", (clk - waiting_time_utili) * 100 / clk);
    fprintf(file, "AvgWTA = %.2f\n", AvgWTA / process_count);
    fprintf(file, "Avg Waiting = %.1f\n", Avg_waiting / process_count);

    // Close the file
    fclose(file);
    // printf("go away\n");
    //  TODO: implement the scheduler.
    //  TODO: upon termination release the clock resources.
    cleanup_memory(&mem);
    kill(getppid(), SIGINT);
    printf("destory me\n");
    destroyClk(true);

    exit(0);
}
void intialize_message_queue()
{
    key_id = ftok("outs/pr_sch_file", 65);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    printf("Message Queue ID = %d\n", msgq_id);
}
void intialize_shared_memory()
{
    key_id2 = ftok("outs/pr_sch_file", 63);
    shmid = shmget(key_id2, 4096, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    else
        printf("\nShared memory ID = %d\n", shmid);
}

int get_data_from_shared_memory()
{
    void *shmaddr = shmat(shmid, NULL, 0);
    int *remaining_time;
    if (shmaddr == (void *)-1)
    {
        perror("Error in attach ");
        exit(-1);
    }
    remaining_time = (int *)shmaddr;
    printf("\nremaining_time = %d\n", *remaining_time);
}
void attach(int shmid, int msg)
{
    void *shmaddr = shmat(shmid, (void *)0, 0);
    if (shmaddr == (void *)-1)
    {
        perror("Error in attach ");
        exit(-1);
    }
    // else
    //{

    // strcpy((int *)shmaddr, msg);
    //}
}
struct PCB *generate_process()
{
    struct process_input_data msgprocess;

    int recieved = msgrcv(msgq_id, &msgprocess, sizeof(struct process_input_data), 0, IPC_NOWAIT); // remove the

    if (recieved == -1 || lastid == msgprocess.id)
    {

        // printf("Error in receiving schedular\n");
        return NULL;
    }
    else
    {
        process_count++;
        struct PCB *newpcb = malloc(sizeof(struct PCB));
        lastid = msgprocess.id;
        newpcb->id = msgprocess.id;
        newpcb->arrivalTime = msgprocess.arrivalTime;
        newpcb->runTime = msgprocess.runTime;
        newpcb->priority = msgprocess.priority;
        newpcb->waitingTime = 0;
        newpcb->remainingTime = msgprocess.runTime;
        newpcb->endTime = -1;
        newpcb->startTime = -1;
        newpcb->stoppedTime = getClk();
        printf("stoooooooooooop %d\n", newpcb->stoppedTime);

        newpcb->state = "";
        newpcb->memory = msgprocess.mem;
        // printing the process data at getclk()
        printf("at time %d process %d arrives with arrival %d memeory %d \n", getClk(), newpcb->id, newpcb->arrivalTime, newpcb->memory);
        // int obj=getClk();
        // while(obj==getClk());
        return newpcb;
    }
}

void round_robin(int Q)
{
    int Finished_Process = -1;
    struct PCB *process = NULL;
    struct PCB *current_process = NULL;
    // int process_during_wait = 0;
    int pid, status;
    int count_Q = 0;
    int count_W = 0;
    file = fopen("inputs_logs/scheduler.log", "a");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        return; // Exit if file cannot be opened
    }

    while (Finished_Process != process_count || !finished || processes_terminated != process_count)
    {
        // printf("start again\n");
        while (1)
        {
            process = generate_process();

            if (process == NULL)
                break;
            fork_allocate(process);
        }
        if (count_Q == Q)
        {
            kill(current_process->pid, SIGSTOP);
            printf("quantum is done\n");
            count_Q = 0;

            // printf("stopped\n");
            current_process->stoppedTime = getClk();
            current_process->state = "stopped";

            fprintf(file, " At time \t%d\tprocess\t%d\t%s\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime);
            Enqueue(current_process);

            current_process = NULL;

            printList();
        }
        if (front != NULL || current_process != NULL)
        {
            // printf("hekko\n");
            if (current_process == NULL)
                current_process = Dequeue2();
            // i need a processing in from of a while loop that is equal to the quantum
            // or the remaining time if it's less than quantum ==> Q+currenttime=finishtime
            // current = front;
            if (count_Q != Q)
            {
                printf("progress on process  %d at time %d \n", current_process->id, getClk());
                printList();
                if (count_Q == 0 && current_process->startTime == -1)
                {
                    // printf("progress\n");
                    current_process->startTime = getClk();
                    current_process->waitingTime = current_process->waitingTime + getClk() - current_process->stoppedTime;
                    current_process->state = "started";
                    kill(current_process->pid, SIGCONT);
                    fprintf(file, " At time \t%d\tprocess\t%d\t%s\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime);
                }
                else if (count_Q == 0 && current_process->startTime != -1)
                {
                    current_process->waitingTime = current_process->waitingTime + getClk() - current_process->stoppedTime;
                    current_process->state = "resumed";
                    kill(current_process->pid, SIGCONT);
                    fprintf(file, " At time \t%d\tprocess\t%d\t%s\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime);
                }
                count_Q++;
                current_process->remainingTime--;
                int obj = getClk();
                while (obj == getClk())
                {
                    process = generate_process();
                    fork_allocate(process);
                }
            }

            if (current_process->remainingTime == 0)
            {
                //--------------------memory
                delocation_forking(current_process);

                current_process->state = "finished";
                count_Q = 0;
                current_process->endTime = getClk();
                if (Finished_Process == -1)
                {
                    Finished_Process = Finished_Process + 2;
                }
                else
                {
                    Finished_Process++;
                }
                int TA = current_process->endTime - current_process->arrivalTime;
                float WTA = (float)TA / current_process->runTime;
                AvgWTA += WTA;
                Avg_waiting += current_process->waitingTime;
                fprintf(file, " At time \t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%f\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime, TA, WTA);

                current_process = NULL;
            }

            // printList();
        }
        if (front == NULL && current_process == NULL)
        {

            if (time != getClk() && getClk() != 0 && (Finished_Process != process_count || !finished))
            {

                time = getClk();
                waiting_time_utili++;
            }
        }
    }
    fclose(file);
}
void tostring(char str[], int num)
{
    int i, rem, len = 0, n;

    n = num;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
}
void handler(int signum)
{

    finished = 1;
}
void increment_processes_terminated(int signum)
{
    processes_terminated++;
}
void forking(struct PCB *process)
{
    int pid;
    if (process != NULL)
    {

        pid = fork();

        if (pid == -1)
            perror("error in fork");

        else if (pid == 0)
        {
            printf("run process id %d\n", process->id);
            int remainingtime = (process->remainingTime);
            char remainingtime_str[10];
            tostring(remainingtime_str, remainingtime);
            execl("outs/process.out", "./outs/process.out", remainingtime_str, NULL);
        }
        else
        {
            process->pid = pid;

            printf("process id %d is forked with process id %d\n", process->id, process->pid);
            if (algorithm_type == 3)
            {
                // process->stoppedTime = getClk();
                Enqueue(process);
            }
            else if (algorithm_type == 4)
            {
                // process->stoppedTime = getClk();
                enqueue(process, process->priority);
                if (current_level > process->priority)
                {
                    printf("current level changed\n");
                    current_level = process->priority;
                }
            }
            else if (algorithm_type == 1)
            {
                // process->stoppedTime = getClk();

                enqueueprio(process, process->remainingTime);
            }
            else
            {
                // process->stoppedTime = getClk();

                enqueueprio(process, process->priority);
            }

            kill(pid, SIGSTOP);
            printf("Parent\n");
        }
    }
}
void multilevel(int Q)
{
    int Finished_Process = -1;
    struct PCB *process = NULL;
    struct PCB *current_process = NULL;

    // int process_during_wait = 0;
    int pid, status;
    int count_Q = 0;
    int count_empty_levels = 0;
    file = fopen("inputs_logs/scheduler.log", "a");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        return; // Exit if file cannot be opened
    }
    intializeQueue();
    while (Finished_Process != process_count || !finished || processes_terminated != process_count)
    {

        while (1)
        {
            process = generate_process();

            if (process == NULL)
                break;
            fork_allocate(process);
        }
        count_empty_levels = 0;
        for (int i = 0; i <= 10; i++)
        {

            current_level = i;
            while (prio[i].queue->front != NULL || current_process != NULL)
            {

                while (1)
                {
                    process = generate_process();

                    if (process == NULL)
                        break;
                    fork_allocate(process);
                }
                if (i > current_level && count_Q == 0)
                {
                    printf("break\n");
                    if (current_level != 0)
                    {
                        i = current_level - 1;
                    }
                    else
                    {
                        i = 10;
                    }
                    printf("level mow %d\n", i);
                    current_level = 11;
                    break;
                }
                if (count_Q == Q)
                {
                    kill(current_process->pid, SIGSTOP);
                    count_Q = 0;
                    current_process->state = "stopped";
                    fprintf(file, " At time \t%d\tprocess\t%d\t%s\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime);
                    current_process->stoppedTime = getClk();

                    if (i != 10)
                    {
                        enqueue(current_process, i + 1);
                        // dequeue(current_process, i);
                    }
                    else
                    {
                        enqueue(current_process, current_process->priority);
                        // dequeue(current_process, i);
                    }
                    current_process = NULL;
                    if (i > current_level)
                    {
                        printf("break\n");
                        if (current_level != 0)
                        {
                            i = current_level - 1;
                        }
                        else
                        {
                            i = 10;
                        }
                        printf("level mow %d\n", i);
                        current_level = 11;
                        break;
                    }
                }
                if (current_process == NULL && prio[i].queue->front != NULL)
                {

                    current_process = dequeue(i);
                    printf("waiting\n");
                }
                if (current_process == NULL && prio[i].queue->front == NULL)
                {
                    break;
                }
                // current_queue = prio[i].queue->front;
                PrintQueue();

                if (count_Q != Q && current_process != NULL)
                {
                    printf("progress on process  %d at time %d \n", current_process->id, getClk());
                    // PrintQueue();
                    if (count_Q == 0 && current_process->startTime == -1)
                    {
                        current_process->waitingTime = current_process->waitingTime + getClk() - current_process->stoppedTime;
                        current_process->startTime = getClk();
                        current_process->state = "started";
                        fprintf(file, " At time \t%d\tprocess\t%d\t%s\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tmem\t%d\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime, current_process->memory);
                    }
                    else if (count_Q == 0 && current_process->startTime != -1)
                    {
                        current_process->waitingTime = current_process->waitingTime + getClk() - current_process->stoppedTime;
                        current_process->state = "resumed";
                        fprintf(file, " At time \t%d\tprocess\t%d\t%s\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tmem\t%d\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime, current_process->memory);
                    }
                    kill(current_process->pid, SIGCONT);
                    // printf("i arrivied at level %d \n", i);
                    count_Q++;
                    current_process->remainingTime--;
                    int obj = getClk();

                    while (obj == getClk())
                    {
                        process = generate_process();
                        fork_allocate(process);
                    }
                }

                if (current_process->remainingTime == 0)
                {

                    //--------------------memory
                    delocation_forking(current_process);

                    count_Q = 0;
                    current_process->state = "finished";
                    current_process->endTime = getClk();
                    if (Finished_Process == -1)
                    {
                        Finished_Process = Finished_Process + 2;
                    }
                    else
                    {
                        Finished_Process++;
                    }
                    int TA = current_process->endTime - current_process->arrivalTime;
                    float WTA = (float)TA / current_process->runTime;
                    AvgWTA += WTA;
                    Avg_waiting += current_process->waitingTime;
                    printf("hi");
                    fprintf(file, " At time \t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tmem\t%d\tTA\t%d\tWTA\t%f\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime, current_process->memory, TA, WTA);
                    current_process = NULL;
                    if (i > current_level)
                    {
                        printf("xbreak\n");
                        printf("hi %d\n", current_level);
                        if (current_level != 0)
                        {
                            i = current_level - 1;
                        }
                        else
                        {
                            i = 10;
                        }
                        current_level = 11;
                        break;
                    }
                }
                // if (current_process != NULL)
                //     kill(current_process->pid, SIGSTOP);
            }

            if (prio[i].queue->front == NULL && current_process == NULL)
            {
                count_empty_levels++;
            }
        }
        if (count_empty_levels == 11)
        {
            if (time != getClk() && getClk() != 0 && (Finished_Process != process_count || !finished))
            {

                time = getClk();
                waiting_time_utili++;
            }
        }
    }
    fclose(file);
}
void SJF()
{
    // initialize memory
    PCB *waiting_process = NULL;
    int Finished_Process = -1;
    struct PCB *process = NULL;
    struct PCB *current_process = NULL;
    int firsttime = 0;
    // int process_during_wait = 0;
    int pid, status;
    // int count_Q = 0;

    file = fopen("inputs_logs/scheduler.log", "a");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        return; // Exit if file cannot be opened
    }

    while (Finished_Process != process_count || !finished || processes_terminated != process_count)
    {
        // printf("start again\n");

        while (1)
        {
            process = generate_process();

            if (process == NULL)
                break;
            fork_allocate(process);
        }

        if (pq_front != NULL || current_process != NULL)
        {
            if (current_process == NULL)
            {
                current_process = dequeueprio();
            }
            printprioQueue();
            printf("Hello %d", current_process->id);
            if (current_process->remainingTime != 0)
            {
                printf("progress on process  %d at time %d \n", current_process->id, getClk());
                printprioQueue();
                if (current_process->startTime == -1)
                {

                    // printf("progress\n");
                    current_process->startTime = getClk();
                    current_process->waitingTime = current_process->startTime - current_process->stoppedTime;
                    printf("start %d stopped %d\n", current_process->startTime, current_process->stoppedTime);
                    current_process->state = "started";
                    fprintf(file, " At time \t%d\tprocess\t%d\t%s\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tmem\t%d\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime, current_process->memory);
                }
                if (current_process->state == "stopped")
                {
                    current_process->waitingTime = current_process->waitingTime + getClk() - current_process->stoppedTime;
                    current_process->state = "resumed";
                    fprintf(file, " At time \t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tmem\t%d\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime, current_process->memory);
                }
                kill(current_process->pid, SIGCONT);
                // count_Q++;
                current_process->remainingTime--;
                int obj = getClk();
                while (obj == getClk())
                {
                    process = generate_process();
                    fork_allocate(process);
                }
            }

            if (current_process->remainingTime == 0)
            {
                // memory
                delocation_forking(current_process);

                current_process->state = "finished";
                // count_Q = 0;
                current_process->endTime = getClk();
                if (Finished_Process == -1)
                {
                    Finished_Process = Finished_Process + 2;
                }
                else
                {
                    Finished_Process++;
                }
                int TA = current_process->endTime - current_process->arrivalTime;
                float WTA = (float)TA / current_process->runTime;
                AvgWTA += WTA;
                Avg_waiting += current_process->waitingTime;
                // printf("thankyoo\n");
                fprintf(file, " At time \t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tmem\t%d\tTA\t%d\tWTA\t%f\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime, current_process->memory, TA, WTA);

                current_process = NULL;
                printprioQueue();
            }
        }
        if (pq_front == NULL && current_process == NULL)
        {
            if (time != getClk() && getClk() != 0 && (Finished_Process != process_count || !finished))
            {
                // fprintf(file, "current time %d time %d\n", getClk(), time);
                time = getClk();
                waiting_time_utili++;
            }
        }
    }
    fclose(file);
}
void HPF()
{

    int Finished_Process = -1;
    struct PCB *process = NULL;
    struct PCB *current_process = NULL;
    // int process_during_wait = 0;
    int pid, status;
    // int current_id = 0;
    //  int count_Q = 0;
    file = fopen("inputs_logs/scheduler.log", "a");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        return; // Exit if file cannot be opened
    }
    while (Finished_Process != process_count || !finished || processes_terminated != process_count)
    {

        // printf("start again\n");
        while (1)
        {
            process = generate_process();

            if (process == NULL)
                break;
            fork_allocate(process);
        }

        if (current_process != NULL && pq_front != NULL)
        {

            if (current_process->priority > pq_front->pcb->priority)
            {
                kill(current_process->pid, SIGSTOP);
                current_process->stoppedTime = getClk();
                current_process->state = "stopped";
                fprintf(file, " At time \t%d\tprocess\t%d\t%s\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime);
                enqueueprio(current_process, current_process->priority);
                current_process = NULL;
            }
        }
        if (pq_front != NULL || current_process != NULL)
        {

            if (current_process == NULL)
            {
                current_process = dequeueprio();
            }
            printprioQueue();
            //         //             current_ptr = pq_front;
            if (current_process->remainingTime != 0)
            {
                printf("progress on process  %d at time %d \n", current_process->id, getClk());
                // printprioQueue();
                if (current_process->startTime == -1)
                {
                    // printf("progress\n");
                    current_process->startTime = getClk();
                    current_process->waitingTime = current_process->startTime - current_process->stoppedTime;
                    // printf("start %d stopped %d\n",current_process->startTime,current_process->stoppedTime);
                    current_process->state = "started";
                    fprintf(file, " At time \t%d\tprocess\t%d\t%s\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime);
                }
                if (current_process->state == "stopped")
                {
                    current_process->waitingTime = current_process->waitingTime + getClk() - current_process->stoppedTime;
                    current_process->state = "resumed";
                    fprintf(file, " At time \t%d\tprocess\t%d\t%s\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime);
                }
                kill(current_process->pid, SIGCONT);

                current_process->remainingTime--;
                int obj = getClk();
                while (obj == getClk())
                {
                    process = generate_process();
                    fork_allocate(process);
                }
            }
            printf("busyyyyyyyyy\n");
            if (current_process->remainingTime == 0)
            {
                //--------------------memory
                delocation_forking(current_process);

                current_process->state = "finished";
                //                 // count_Q = 0;
                current_process->endTime = getClk();
                if (Finished_Process == -1)
                {
                    Finished_Process = Finished_Process + 2;
                }
                else
                {
                    Finished_Process++;
                }
                int TA = current_process->endTime - current_process->arrivalTime;
                float WTA = (float)TA / current_process->runTime;
                AvgWTA += WTA;
                Avg_waiting += current_process->waitingTime;
                // printf("thankyoo\n");
                fprintf(file, " At time \t%d\tprocess\t%d\t%s\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%f\n", getClk(), current_process->id, current_process->state, current_process->arrivalTime, current_process->runTime, current_process->remainingTime, current_process->waitingTime, TA, WTA);
                current_process = NULL;
            }

            printprioQueue();
        }
        if (pq_front == NULL && current_process == NULL)
        {
            if (time != getClk() && getClk() != 0 && (Finished_Process != process_count || !finished))
            {
                //    fprintf(file, "current time %d time %d\n", getClk(), time);
                time = getClk();
                waiting_time_utili++;
            }
        }
    }
    fclose(file);
}
void fork_allocate(PCB *process)
{
    if (process != NULL)
    {
        //     forking(process);
        if (allocate_memory(&mem, process->memory, process->id, getClk()) == true)
        {
            forking(process);
            printQueue_final(waiting_queue);
            print_memory(&mem);
        }
        else
        {
            printQueue_final(waiting_queue);
            enQueue_final(waiting_queue, process);
            printQueue_final(waiting_queue);
        }
    }
}

void delocation_forking(PCB *current_process)
{
    printf("process with id %d Finished  at time %d\n", current_process->id, getClk());
    printQueue_final(waiting_queue);
    deallocate_memory(&mem, current_process->id, getClk(),current_process->memory);
    //-----------------------memory
    if (!isEmpty(waiting_queue))
    {
        PCB *next_process = frontQueue_final(waiting_queue);
        if (allocate_memory(&mem, next_process->memory, next_process->id, getClk()))
        {
            next_process = deQueue_final(waiting_queue);
            printf("Process %d allocated memory from waiting queue at time %d\n", next_process->id, getClk());
            forking(next_process); // Or handle this process
        }
    }
}
