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

void insertar_en_queue(Queue *q, Process *p) {
    if (q->size < q->capacidad) {
        q->procesos[q->size] = p;
        q->size++;
    }
}

void eliminar_de_queue(Queue *q, Process *p) {
    for (int i = 0; i < q->size; i++) {
        if (q->procesos[i] == p) {
            for (int j = i; j < q->size - 1; j++) {
                q->procesos[j] = q->procesos[j + 1];
            }
            q->size--;
            break;
        }
    }
}

void liberar_queue(Queue *q) {
    free(q->procesos); 
    free(q);           
}

int comparacion_edf(const void *a, const void *b) {
    Process *p1 = *(Process**)a;
    Process *p2 = *(Process**)b;
    if (p1->t_deadline != p2->t_deadline) { // menor deadline
        return p1->t_deadline - p2->t_deadline;
    }
    return p1->pid - p2->pid; // en caso de empate, menor PID
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

    // Completa el codigo. Exito :D

    while (procesos_terminados < nProcesses) {
        
        for (int i = 0; i < nProcesses; i++) {
            if (todos_los_procesos[i]->t_inicio == tick) {
                todos_los_procesos[i]->state = READY;
                insertar_en_queue(cola_high, todos_los_procesos[i]);
            }
        }

        for (int i = 0; i < nProcesses; i++) {
            if (todos_los_procesos[i]->state == WAITING) {
                todos_los_procesos[i]->progreso_io_actual++;
                if (todos_los_procesos[i]->progreso_io_actual == todos_los_procesos[i]->io_wait) {
                    todos_los_procesos[i]->state = READY;
                    todos_los_procesos[i]->progreso_io_actual = 0;
                    insertar_en_queue(cola_high, todos_los_procesos[i]); 
                }
            }
        }

        for (int i = cola_low->size - 1; i >= 0; i--) { // hacia atrás para no matar el indice al eliminar
            Process *p = cola_low->procesos[i];
            if ((tick - p->t_lcpu) > agingThreshold) {
                eliminar_de_queue(cola_low, p);
                p->quantum_consumido = 0; // en el enunciado dice que se resetea el quantum de un proceso solo si es que vuelve a la cola High desde la Low, no si es que vuelve de la CPU a la High
                insertar_en_queue(cola_high, p);
            }
        }

        for (int i = 0; i < nProcesses; i++) {
            Process *p = todos_los_procesos[i];
            if (tick >= p->t_deadline && (p->state == READY || p->state == WAITING)) {
                p->state = DEAD;
                procesos_terminados++; 
                eliminar_de_queue(cola_high, p);
                eliminar_de_queue(cola_low, p);
            }
        }

        // Falta implementar
        // 1. Re-ordenar colas por deadline, EDF (punto 5 del flujo del scheduler)
        // El punto 2. y 3. son parte del punto 6 y 7 del flujo del scheduler por si acaso
        // 2. Trabajar con el cpu_running. Por ejemplo, si un proceso que está RUNNING en la CPU termina su ráfaga actual y aún le quedan ráfagas por hacer, entonces el Scheduler dice: "Terminaste esta ráfaga de CPU, ahora anda a hacer I/O", lo cambia a estado WAITING, lo saca de la CPU, y guarda su t_lcpu = tick (la última vez que salió de la CPU)
        // 2.1 Lo otro, tener en consideración que si cpu_running->io_wait == 0, entonces el proceso vuelve a Ready y a la cola High
        // 3. Si CPU está vacía, sacar de High o Low
        // 4. Registrar los tiempos Turnaround, Response y waiting.
        // 5. Generar el output .csv

        for (int i = 0; i < nProcesses; i++) {
            if (todos_los_procesos[i]->state == READY) {
                todos_los_procesos[i]->waiting_time++;
            }
        }
        tick++;
    }

    fclose(out);
    liberar_queue(cola_high);
    liberar_queue(cola_low);
    for (int i = 0; i < nProcesses; i++) {
        free(todos_los_procesos[i]); 
    }
    free(todos_los_procesos);
    
    return 0;
};