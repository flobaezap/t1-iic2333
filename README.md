# T1 - IIC2333

## Estructuras de Datos Implementadas

### 1. Process Control Block (`struct Process`)
Modela toda la información de un proceso. Se divide en:
* Datos leídos directamente del input (`pid`, `t_inicio`, `t_cpu_burst`, `n_bursts`, `io_wait`, `t_deadline`).
* Variables que manejan el estado del proceso en el tiempo real de la simulación. Incluye el progreso actual de ráfagas (`progreso_rafaga_actual`), el tiempo esperando I/O (`progreso_io_actual`), el quantum consumido, y la última vez que el proceso dejó la CPU (`t_lcpu`).
* Métricas para el output final (`first_time_cpu`, `waiting_time`, `interrupciones`, `turnaround_time`).
* Además, se definió un `enum` que tiene los diferentes estados posibles de un proceso: `READY`, `RUNNING`, `WAITING`, `FINISHED`, y `DEAD`. 

### 2. Colas (`struct Queue`)
Se implementó una estructura de cola dinámica que guarda **punteros a procesos** (`Process **procesos`), lo que permite mover procesos entre la CPU, I/O, la cola High y la cola Low moviendo solo las referencias y no copiando el `struct` completo.

---

## Flujo del Scheduler

Tenemos de base un ciclo `while` que avanza de a un `tick` hasta que la variable `procesos_terminados` es igual al total de procesos. Por cada `tick`, se realizan las siguientes acciones en orden:

1. **Llegadas:** Se revisa el arreglo de procesos. Si el `t_inicio` de un proceso es igual al `tick` actual, el proceso cambia a estado `READY` y se inserta en la cola High.
2. **Retorno de I/O:** Se revisan todos los procesos en estado `WAITING`. Se incrementa su tiempo en I/O y, si alcanzan su límite (`io_wait`), vuelven a estado `READY`, se resetea su contador de progreso I/O y son devueltos a la cola High cumpliendo la regla del **I/O Boost**.
3. **Ascenso (Aging):** Para evitar que procesos intensivos sufran inanición en la cola Low, se itera sobre ella. Si el proceso supera el `agingThreshold` desde la última vez que estuvo en la CPU (`t_lcpu`), se saca de Low, se resetea su quantum, y se pasa a la cola High.
4. **Verificación de Deadlines:** Todo proceso en estado `READY` o `WAITING` cuyo tiempo superó el `t_deadline` se actualiza a estado `DEAD`. Se elimina de cualquier cola en la que esté, se calcula su *Turnaround Time* y se cuenta como un proceso terminado.
5. **Ordenamiento EDF:** Se reordenan ambas colas utilizando `qsort` en conjunto con la función `comparacion_edf`. Esto asegura que el proceso con el menor deadline, o menor PID en caso de empate, quede siempre en la posición `[0]`.
6. **Ejecución en CPU:** utilizamos el puntero `cpu_running`. Se aumenta su progreso de ráfaga y se verifica:
   * Si terminó su última ráfaga (pasa a `FINISHED`).
   * Si llegó a su deadline mientras corría (pasa a `DEAD`).
   * Si terminó una ráfaga intermedia (pasa a `WAITING` por I/O, o vuelve a `READY` si su `io_wait` es 0).
   * Si consumió todo su quantum (pasa a `READY` en la cola Low).
   * Si debe ser interrumpido por un proceso con un deadline más urgente (Preemption).
7. **Dispatch:** Si la CPU queda vacía tras el paso 6, se saca al proceso de mayor prioridad de la cola High. Si High está vacía, se saca de la cola Low. Al proceso elegido se le cambia el estado a `RUNNING` y se registra su `first_time_cpu` si es la primera vez que se ejecuta.
8. **Métricas Temporales:** Al final de cada tick, a todos los procesos que estén en estado `READY` o `WAITING` se les suma 1 a su `waiting_time`.

---

## Sobre el Manejo de Memoria

El simulador garantiza **0 leaks de memoria** al finalizar su ejecución:
* Se crearon las funciones `crear_queue` y `liberar_queue`.
* Todos los procesos se guardan inicialmente en el arreglo `todos_los_procesos`.
* Cuando termina el `while`, el programa imprime el archivo CSV y luego libera los arreglos internos de las colas, las estructuras de las colas, cada proceso individual pedido con `malloc` y el arreglo de procesos.