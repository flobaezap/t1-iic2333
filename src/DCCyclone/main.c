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
    int turnaround_time;
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

// decide si el proceso debe ser preemptado por prioridad
bool debe_preemptar(Process *cpu_running, Queue *cola_high, Queue *cola_low, bool comes_from_high) {
    if (cpu_running == NULL || cola_high->size == 0) return false;
    if (cola_high->size > 0) {    
        if (!comes_from_high) return true; 
        Process *top = cola_high->procesos[0];
        if (top->t_deadline < cpu_running->t_deadline) return true;
        if (top->t_deadline == cpu_running->t_deadline && top->pid < cpu_running->pid) return true;
    }

    if (!comes_from_high && cola_low->size > 0) {   
        Process *top = cola_low->procesos[0];
        if (top->t_deadline < cpu_running->t_deadline) return true;
        if (top->t_deadline == cpu_running->t_deadline && top->pid < cpu_running->pid) return true;
    }
    
    return false;
}

// saca proceso de la CPU a donde estaba
void preemptar_cpu(Process **cpu_running, Queue *cola_high, Queue *cola_low, bool comes_from_high, int tick) {
    Process *p = *cpu_running;
    p->state = READY;
    p->interrupciones++;
    p->t_lcpu = tick;
    if (comes_from_high) {
        insertar_en_queue(cola_high, p);
    } else {
        insertar_en_queue(cola_low, p);
    }
    *cpu_running = NULL;
}

// elige proximo proceso a ejecutar
void dispatch_cpu(Process **cpu_running, Queue *cola_high, Queue *cola_low, bool *comes_from_high, int tick) {
    if (*cpu_running != NULL) return;
    Process *elegido = NULL;
    if (cola_high->size > 0) {
        elegido = cola_high->procesos[0];
        eliminar_de_queue(cola_high, elegido);
        *comes_from_high = true;
    } else if (cola_low->size > 0) {
        elegido = cola_low->procesos[0];
        eliminar_de_queue(cola_low, elegido);
        *comes_from_high = false;
    }
    if (elegido != NULL) {
        elegido->state = RUNNING;
        if (elegido->first_time_cpu == -1) {
            elegido->first_time_cpu = tick;
        }
        *cpu_running = elegido;
    }
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
        p->turnaround_time = 0;
        todos_los_procesos[i] = p;
    }

    fclose(file);
    Queue *cola_high = crear_queue(nProcesses);
    Queue *cola_low = crear_queue(nProcesses);
    Process *cpu_running = NULL; 
    int procesos_terminados = 0; 
    int tick = 0;
    bool comes_from_high = true;

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
                p->turnaround_time = tick - p->t_inicio;
                procesos_terminados++; 
                eliminar_de_queue(cola_high, p);
                eliminar_de_queue(cola_low, p);
            }
        }

        // re-ordenar colas por deadline 
        qsort(cola_high->procesos, cola_high->size, sizeof(Process*), comparacion_edf);
        qsort(cola_low->procesos, cola_low->size, sizeof(Process*), comparacion_edf);

        // ejecucion en cpu
        if (cpu_running != NULL) {
            cpu_running->progreso_rafaga_actual++;
            if (comes_from_high) {
                cpu_running->quantum_consumido++;
            }

            bool termino_rafaga = (cpu_running->progreso_rafaga_actual == cpu_running->t_cpu_burst);
            bool es_ultima_rafaga = (cpu_running->rafagas_completadas + 1 == cpu_running->n_bursts);
            bool llego_deadline = (tick >= cpu_running->t_deadline);
            bool fin_quantum = (comes_from_high && cpu_running->quantum_consumido >= q);

            // ejecuta rafaga entera
            if (termino_rafaga && es_ultima_rafaga) {
                cpu_running->state = FINISHED;
                cpu_running->turnaround_time = tick - cpu_running->t_inicio;
                cpu_running->t_lcpu = tick;
                procesos_terminados++;
                cpu_running = NULL;
            }
            // deadline alcanzado a mitad de rafaga
            else if (llego_deadline) {
                cpu_running->state = DEAD;
                cpu_running->interrupciones++;
                cpu_running->turnaround_time = tick - cpu_running->t_inicio;
                cpu_running->t_lcpu = tick;
                procesos_terminados++;
                cpu_running = NULL;
            }
            // fin de rafaga
            else if (termino_rafaga) {
                cpu_running->rafagas_completadas++;
                cpu_running->progreso_rafaga_actual = 0;
                if (!comes_from_high) {
                    cpu_running->quantum_consumido = 0; 
                }
                if (cpu_running->io_wait == 0) {
                    cpu_running->state = READY;
                    insertar_en_queue(cola_high, cpu_running);
                } else {
                    cpu_running->state = WAITING;
                }
                cpu_running->t_lcpu = tick;
                cpu_running = NULL;
            } 
            // consume todo el quantum
            else if (fin_quantum) {
                cpu_running->state = READY;
                cpu_running->interrupciones++;
                insertar_en_queue(cola_low, cpu_running);
                cpu_running->t_lcpu = tick;
                cpu_running = NULL;
            } 
            // interrupcion por prioridad
            else if (debe_preemptar(cpu_running, cola_high, cola_low,comes_from_high)) {
                preemptar_cpu(&cpu_running, cola_high, cola_low, comes_from_high, tick);
            }
        }

        // si cpu esta libre, sacar de high o low
        dispatch_cpu(&cpu_running, cola_high, cola_low, &comes_from_high, tick);

        for (int i = 0; i < nProcesses; i++) {
            Process *p = todos_los_procesos[i];
            if (tick < p->t_inicio) continue; // proceso aun no nace
            if (p->state == READY || p->state == WAITING) {
                p->waiting_time++;
            }
        }
        tick++;
    }

    // imprimir output csv 
    for (int i = 0; i < nProcesses; i++) {
        Process *p = todos_los_procesos[i];
        int response_time = (p->first_time_cpu == -1) ? 0 : (p->first_time_cpu - p->t_inicio);
        const char *state_str = (p->state == FINISHED) ? "FINISHED" : "DEAD";
        fprintf(out, "%s,%d,%s,%d,%d,%d,%d\n", 
                p->name, 
                p->pid, 
                state_str, 
                p->interrupciones, 
                p->turnaround_time, 
                response_time, 
                p->waiting_time);
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