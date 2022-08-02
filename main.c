#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

int first_run = 1;
char homeDIR[1024];

struct Node
{
    pid_t p_id;
    char **args;
    int active;
    int size;
    struct Node *next;
};

struct Node* head = NULL;
struct Node* head_curr = NULL;


void Display_Screen();
void Shell_Prompt();
int cmd_exec(char **argss);
int cmd_handler(char **args);


int Change_Directory(char **args, int size);
void sys_cmd(char **args, int b, int size);
void hist_run(int num); 
void HISTn(int n);

void Display_all();
void Display_BRIEF();   
void Display_FULL();  

void append_proc(char **args, pid_t p_id, int size);
int cnt_cmd(char **args);  
int count_args(); 
void rev();

void HandleSigint(int sig);
void HandleChildkill(int sig);
void d_node(pid_t pid);
void killp();
void clear_curr();
void killallC();
void append_current(char **args, pid_t pid, int size);
void Display_current();



/* for background process -----------
struct Node* head_current = NULL;
void append_current(char **args, pid_t p_id);



---------------------------------*/


int main(int argc, char **agrv, char **envp)
{
    signal(SIGINT, HandleSigint);
    signal(SIGCHLD, HandleChildkill);




    Display_Screen();
    int flag = 1;
    while(flag)
    {
        clear_curr();
        Shell_Prompt();

        char cmd_line[1024]; // input given by user
        char *token[100];  // tokens in cmd_line

        int count = 0; // count of the token in each cmd_line
        int exit_e;

        memset(cmd_line, '\0', 1024);
        fgets(cmd_line, 1024, stdin);

        char *temp_tkn = strtok(cmd_line, " \n\t\r");
        if(temp_tkn == NULL)
        {
            continue;
        }
        else if(strcmp(temp_tkn, "EXEC") == 0)
        {
            while(temp_tkn != NULL)
            {
                token[count] = temp_tkn;
                count ++;
                temp_tkn = strtok(NULL, " \n\t\r");
            }
            char *argss[count];

            for(int i=0; i<count-1; i++)
            {
                argss[i] = strdup(token[i+1]);
            }
            argss[count-1] = NULL;

            exit_e = cmd_exec(argss);
        }
        else
        {
            while(temp_tkn != NULL)
            {
                token[count] = temp_tkn;
                count ++;
                temp_tkn = strtok(NULL, " \n\t\r");
            }
            char *args[count+1];
            for(int i=0; i<count; i++)
            {
                args[i] = strdup(token[i]);
            }
            args[count] = NULL;
            exit_e = cmd_handler(args);
        }

        if(exit_e == -1)
        {
            flag = 0;
            break;
        }
    }

    // printf("\n\nEXITING FROM PROGRAM BY USER !!\n\n");

    printf("EXITING FROM PROGRAM BY USER !!, \nAll rights reserved to Chandan(2019UCS0094😎) :)\n");
    exit(0);
    return 0;
}

void Display_Screen()
{
    printf("\n\n\t----------------------------------------\n");
    printf("\t             User_Interactive_Shell_starts\n");
    printf("\t--------------------------------------------\n");
    printf("\n\n");
}

//------shell prompt
void Shell_Prompt()
{
    char *user, cwd[1024], hostn[1204] = "";

    user =  getenv("USER");
    gethostname(hostn, sizeof(hostn));
	getcwd(cwd, sizeof(cwd));

    //printf("%s\n\n %s\n\n %s\n\n", user, hostn, cwd);

    if(first_run) 
    {
        getcwd(homeDIR, sizeof(homeDIR));
        first_run = 0;
    }

    if(strncmp(cwd, homeDIR, strlen(homeDIR)) == 0) 
    {
        char *truncatedcwd = cwd + strlen(homeDIR);
        printf("<%s@%s:~%s> ", user, hostn, truncatedcwd);
    } 
    else 
        printf("<%s@%s:%s> ", user, hostn, cwd);
    

}

//--------user command EXEC
int cmd_exec(char** argss)
{   if(strcmp(argss[0], "exit")==0)
    {
        return -1;
    }
    pid_t pid = fork();
    if(pid == -1)
    {
        printf("child process is not created");    
        return 0;    
    }

    else if(pid == 0)
    {
        int ee = execvp(argss[0], argss);
        if(ee == -1)
        {
            printf("command not found!\n");
            return 0;
        }
    }
    else
    {
        int x = wait(NULL);
        int size = cnt_cmd(argss);
        append_proc(argss, x, size);
    }
    return 0;
}

//---------input command handling
int cmd_handler(char **args)
{
    int size = cnt_cmd(args);
    if(strcmp(args[0], "exit") == 0)
    {
        return -1;
    }
    else if(strcmp(args[0], "STOP") == 0)
    {
        return -1;
    }
    else if(strcmp(args[0], "HISTORY") == 0)
    {
        append_proc(args, getpid(), size);
        if(args[1] == NULL)
        {
            printf("input is incomplete, FULL/BRIEF , enter full cmd\n");
            return 0;
        }
        else if(strcmp(args[1], "BRIEF") == 0)
        {
            Display_BRIEF();
        }
        else if(strcmp(args[1], "FULL") == 0)
        {
            Display_FULL();
        }
        else
        {
            printf("Wrong input with HISTORY\n");
        }
    }
    else if(strcmp(args[0], "pid") == 0)
    {
        
        if(args[1] == NULL)
        {
            append_proc(args, getpid(), size);
            int child_pid = getpid();
            printf("command name: ./a.out  process id: %d\n", child_pid);
            return 0;
        }
        else if(strcmp(args[1], "all") == 0)
        {
            append_proc(args, getpid(), size);
            Display_all(); 
            return 0;
        }
        else if(strcmp(args[1], "current") == 0)
        {
            Display_current();
        }
        else
        {
            printf("invalid arguments with pid!! \n");
            return 0;
        }
    }
    else if(strncmp(args[0], "!HIST", 5) == 0)
    {
        char *temp = strdup(args[0]);
        char n_temp[10];
        for(int i=0; temp[i] != '\0'; i++)
        {
            n_temp[i] = temp[i+5];
        }
        int p = atoi(n_temp);
        //printf("%d\n", p);
        if(strcmp("0", n_temp)==0)
        {
            return 0;
        }
        else if(p != 0)
        {
            hist_run(p);
            append_proc(args, getpid(), size);
            return 0;
            //printf("%d\n", p);

        }
        //!HIST(N1, N2, N3, N4) -- ??
        else if(strncmp("!HIST(", args[0], 6)==0)
        {
            int j=0;
            char cp[10];
            for(int i=0; args[j][i] != '\0'; i++)
            {
                cp[i] = args[j][i+6];
            }
            j++;
            int x = atoi(cp);
            hist_run(x);
            //printf("%d ", x);
            while(j != cnt_cmd(args)-1)
            {
                char cpp[10];
                for(int i=0; args[j][i] != ','; i++)
                {
                    cpp[i] = args[j][i];
                }
                int pp = atoi(cpp);
                hist_run(pp);
                //printf("%d ", pp);
                j++;
            }
            char cppp[10];
            for(int i=0; args[j][i] != ')'; i++)
            {
                cppp[i] = args[j][i];
            }
            int ppp = atoi(cppp);
            hist_run(ppp);
            //printf("%d ", ppp);
            printf("\n");
        }
        else
        {
            printf("wrong input with !HIST  !!\n");
            return 0;
        }
    }
    else if(strncmp(args[0], "HIST", 4) == 0)
    {
        char *temp = strdup(args[0]);
        char n_temp[10];
        for(int i=0; temp[i] != '\0' ; i++)
        {
            n_temp[i] = temp[i+4];
        }
        int p = atoi(n_temp);
        if(strcmp("0", n_temp) == 0)
        {
            //printf("this is just for test ,, remove this line\n");
            return 0;
        }
        else if(p==0)
        {
            printf("wrong input with HIST !!\n");
            return 0;
        }
        else
        {
            HISTn(p);
            return 0;
        }
    }
    else if(strcmp(args[0], "pwd") == 0)
    {
        append_proc(args, getpid(), size);
        char *res = malloc(100*sizeof(char)); 
        printf("%s\n", getcwd(res,100));
        free(res);
    }
    else if(strcmp(args[0], "cd") == 0)
    {
        Change_Directory(args, size);
    }
    else
    {
        //sys_cmd(args, 0);
        int b =0;
        if(strcmp(args[size-1], "&")==0)
        {
            b = 1;
            args[size-1] = NULL;
        }

        sys_cmd(args, b, size);

    }
    return 0;
}

//---------HISTORY BRIEF
void Display_BRIEF()
{
    if(head == NULL)
    {
        printf("No process has been executed yet ! \n");
    }
    else
    {
        struct Node *t_node = head;
        for(int i=1; t_node != NULL; i++)
        {
            printf("%d. %s\n", i, (t_node -> args)[0]);
            t_node = t_node->next;
        }
    }
}

//-------- HISTORY FULL
void Display_FULL()
{
    if(head == NULL)
    {
        printf("No process has been executed yet ! \n");
    }
    else
    {
        struct Node *t_node = head;

        for(int i=1; t_node != NULL; i++)
        {
            int c = (t_node->size);
            printf("%d. ", i);
            for(int j=0; j<c; j++)
            {
                printf("%s ", (t_node->args)[j]);
            }
            printf("\n");
            t_node = t_node->next;
        }
    }
}

///---------pid all
void Display_all()
{
    if(head == NULL)
    {
        printf("NO process has beesn executed yet ! \n");
        return;
    }
    struct Node *t_node = head;
    for(int i=1; t_node != NULL; i++)
    {
        printf("%d.Command name: ", i);
        for(int j=0; j<(t_node->size); j++)
        {
            printf("%s ", (t_node->args)[j]);
        }
        printf("process id: %d\n", t_node->p_id);

        t_node = t_node->next;
    }
}

//-------cd
int Change_Directory(char **args, int size)
{
    if(args[1] == NULL)
    {
        // chdir(getenv("HOME"));
        chdir(homeDIR);
        append_proc(args, getpid(), size);
        return 1;
    }
    else
    {
        int dc = chdir(args[1]);
        if(dc == -1)
        {
            printf(" %s: No such file or directory\n", args[1]);
            return -1;
        }
    }
    append_proc(args, getpid(), size);
    return 0;
}

//---------executing system commands
/*
void sys_cmd(char **args)
{
    int status;
    pid_t pid = fork();
    if(pid == -1)
    {
        printf("child process is not created");    
        return;    
    }

    else if(pid == 0)
    {

        int ee = execvp(args[0], args);
        if(ee == -1)
        {
            printf("command not found!\n");
            return;
        }
    }
    else
    {
        int x = wait(NULL);
        append_proc(args, x);
        
    }
}
*/

void sys_cmd(char **args, int b, int size)
{
    pid_t pid;
    int status;
    pid = fork();
    if(pid == -1)
    {
        printf("child process not created\n");
        return;
    }
    else if(pid == 0)
    {
        if(b == 1)
        {
            int pgid = setpgid(0, 0);
            if(pgid == -1)
            {
                printf("background process not executed \n");
                return;
            }
        }
        int ee = execvp(args[0], args);
        if(ee < 0)
        {
            printf("command not found!\n");
            return;
        }
    }
    else
    {
        if(b == 1)
        {
            args[size-1] = "&";
        }
        append_proc(args, pid, size);


        if(b != 1)
        {
            waitpid(pid, &status, 0);
        }
        else
        {
            printf("background process is running with pid: %d\n", pid );
            append_current(args, pid, size);
        }
    }
}


//------appending the args and pid to linked list
void append_proc(char **args, pid_t p_id, int size)
{
    struct Node* t_node = head;
    struct Node *New_node = malloc(sizeof(struct Node));

    int count = size;

    New_node -> size = size;

    New_node -> p_id = p_id;
    New_node -> args = (char **)malloc(sizeof(char) * (count+1));
    
    for(int i=0; i<count; i++)
    {
        New_node->args[i] = (char *)malloc(sizeof(char)* sizeof(strlen(args[i])+1));
        strcpy(New_node -> args[i], args[i]);
    }

    New_node -> next = NULL;
    if(t_node == NULL)
    {
        head = New_node;
    }
    else
    {
        for(int i=0; t_node->next != NULL; i++)
        {
            t_node = t_node->next;
        }
        t_node->next = New_node;
    }
}

//-------counting no. of tokens in a args
int cnt_cmd(char **args)
{
    int cnt = 0;
    for(int i=0; args[i] != NULL; i++)
    {
        cnt++;
    }

    return cnt;
}

//--------counting no. of args in linked list
int count_args()
{
    if(head == 0)
    {
        return 0;
    }
    
    int c;
    struct Node *t_node = head;
    for(int i=0; t_node != NULL; i++)
    {
        t_node = t_node ->next;
        c++;
    }
    return c;
}

//---------!HISTN & !HIST(N1, N2, N3...)
void hist_run(int num)
{
    int c = count_args();
    
    if(num > c)
    {
        printf("total no. of executed process is less than %d\n", num);
        printf("number should be less than %d \n", c+1);
        return;
    }
    else
    {
        struct Node *t_node = head;
        int x=1;
        
        while(x != num)
        {
            t_node = t_node -> next;
            x++;
        }
        cmd_handler(t_node -> args);
    }
}

//---------executing HISTN user command
void HISTn(int n)
{
    if(head == NULL)
    {
        printf("NO process has beesn executed yet ! \n");
        return;
    }
    rev();
    struct Node *t_node = head;
    // for(int i=0; t_node->next != NULL; i++)
    // {
    //     t_node = t_node->next;
    // }

    int j=1;
    for(j=1; j<=n; j++)
    {
        printf("%d. ", j);
        printf("%s\n", (t_node->args)[0]);
        t_node = t_node->next;

    }
    rev();
    
}


//------to reverse a linked list
void rev()
{
    if(head == NULL)
    {
        return;
    }
    struct Node* p = NULL;
    struct Node* c = head;
    struct Node* next = NULL;


    while(c != NULL)
    {
        next = c -> next;
        c -> next = p;

        p = c;
        c = next;
    }
    head = p;

}

//-----for ctrl+c
void HandleSigint(int sig)
{
    printf("Interrupted by user!\n");
}


void HandleChildkill(int sig)
{
    pid_t pid;
    int status;

    struct Node* t_node = head_curr;

    while(t_node != NULL)
    {
        pid = waitpid(t_node->p_id, &status, WNOHANG);
        if(pid == -1)
        {
            printf("child process terminated with pid : %d\n", t_node->p_id);
            t_node->active = 0;
        }
        else if(pid != 0 && (WIFEXITED(status) || WIFSIGNALED(status) || WCOREDUMP(status)))
        {
            printf("child process terminated with pid : %d\n", t_node->p_id);
            t_node->active = 0;
        }
        t_node = t_node->next;
    }
    return;

}

void d_node(pid_t pid)
{
    struct Node* t_node = head_curr;

    if(t_node != NULL && t_node->p_id == pid)
    {
        head_curr = head_curr -> next;
        free(t_node);
        return;
    }
    else
    {
        while(t_node != NULL && t_node -> p_id != pid)
        {
            t_node = t_node->next;
        }
        if(t_node == NULL)
        {
            return;
        }
        free(t_node);
    }
}

void clear_curr()
{
    struct Node* t_node = head_curr;
    while(t_node != NULL)
    {
        if(t_node->active == 0)
        {
            d_node(t_node->p_id);
        }
        t_node = t_node->next;
    }
    return;
}

void killp()
{
    struct Node* t_node = head_curr;
    while(t_node != NULL)
    {
        if(t_node -> active == 0)
        {
            d_node(t_node->p_id);
        }
        t_node = t_node->next;
    }
    return;
}

void killallC()
{
    struct Node* t_node = head_curr;
    if(t_node != NULL)
    {
        printf("process running in background\n");
        printf("background process is terminating\n");
    }

    while(t_node != NULL)
    {
        kill(t_node->p_id, SIGKILL);
        t_node = t_node->next;
    }
}



void append_current(char **args, pid_t pid, int size)
{
    //printf("in curr appending funtion\n\n");


    struct Node* t_node = head_curr;
    struct Node* temp = malloc(sizeof(struct Node));

    temp->p_id = pid;
    temp->size = size;
    temp->active = 1;
    temp->args = (char **)malloc(sizeof(char) * (size+1));

    for(int i=0; i<size; i++)
    {
        temp->args[i] = (char *)malloc(sizeof(char) * sizeof(strlen(args[i])+1));
        strcpy(temp->args[i], args[i]);
    }
    temp -> next = NULL;

    if(t_node == NULL)
    {
        head_curr = temp;
    }
    else
    {
        for(int i=0; t_node!=NULL; i++)
        {
            t_node = t_node->next;
        }
        t_node -> next = temp;
    }


}


void Display_current()
{
    struct Node* t_node = head_curr;
    int j = 1;
    for(int i=0; t_node != NULL; i++)
    {
        if(t_node->active == 1)
        {
            printf("%d. Command Name: %s process id: %d\n", j, t_node->args[0], t_node->p_id);
            j++;
        }
        t_node = t_node->next;
    }
    if(j==1)   printf("No background process! \n");

    return;
}