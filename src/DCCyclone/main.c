#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    READY,
    RUNNING,
    WAITING,
    FINISHED,
    DEAD
} State;

typedef struct {
    char name[20];
    int pid;
    int t_inicio;      
    int t_cpu_burst;      
    int n_bursts;      
    int io_wait;         
    int t_deadline;        
    State state;
    int rafagas_completadas;       
    int progreso_rafaga_actual; 
    int progreso_io_actual;   
    int quantum_consumido;       
    int t_lcpu;          
    int first_time_cpu;         
    int waiting_time;           
    int interrupciones;          
} Process;

typedef struct {
    Process **procesos; 
    int size;            
    int capacidad;        
} Queue;

Queue* crear_queue(int capacidad) {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    q->procesos = (Process**)malloc(sizeof(Process*) * capacidad);
    q->size = 0;
    q->capacidad = capacidad;
    return q;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Error al abrir archivo de entrada\n");
        return 1;
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        printf("Error al abrir archivo de salida\n");
        return 1;
    }

    int q, agingThreshold, nProcesses;

    fscanf(file, "%d", &q);
    fscanf(file, "%d", &agingThreshold);
    fscanf(file, "%d", &nProcesses);

    Process **todos_los_procesos = (Process**)malloc(sizeof(Process*) * nProcesses);
    char name[20];
    int pid, start, burstTime, burstQty, ioTime, deadline;

    for (int i = 0; i < nProcesses; i++) {
        fscanf(file, "%s %d %d %d %d %d %d",
               name, &pid, &start, &burstTime,
               &burstQty, &ioTime, &deadline);
        
        Process *p = (Process*)malloc(sizeof(Process));
        strcpy(p->name, name);
        p->pid = pid;
        p->t_inicio = start;
        p->t_cpu_burst = burstTime;
        p->n_bursts = burstQty;
        p->io_wait = ioTime;
        p->t_deadline = deadline;
        p->rafagas_completadas = 0;
        p->progreso_rafaga_actual = 0;
        p->progreso_io_actual = 0;
        p->quantum_consumido = 0;
        p->t_lcpu = 0;
        p->first_time_cpu = -1;
        p->waiting_time = 0;
        p->interrupciones = 0;
        todos_los_procesos[i] = p;
    }

    fclose(file);
    Queue *cola_high = crear_queue(nProcesses);
    Queue *cola_low = crear_queue(nProcesses);
    Process *cpu_running = NULL; 
    int procesos_terminados = 0; 
    int tick = 0;

    fclose(file);

    // Completa el codigo. Exito :D
};