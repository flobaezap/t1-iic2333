# T1 - IIC2333

## Estructuras de Datos Implementadas

### 1. El Process Control Block (`struct Process`)
Modela toda la información de un proceso. Se divide en:
* Datos leídos directamente del input (`pid`, `t_inicio`, `t_cpu_burst`, `n_bursts`, `io_wait`, `t_deadline`).
* Variables que manejan el estado del proceso en el tiempo real de la simulación. Incluye el progreso actual de ráfagas (`progreso_rafaga_actual`), el tiempo esperando I/O (`progreso_io_actual`), el quantum consumido, y la última vez que el proceso dejó la CPU (`t_lcpu`).
* Métricas para el output final (`first_time_cpu`, `waiting_time`, `interrupciones`).
* Además, se definió un `enum` que tiene los diferentes estados posibles de un proceso, como `READY`, `RUNNING`, `WAITING`, `FINISHED`, y `DEAD`. (Los procesos que aún no nacen simplemente no entran a las colas hasta que el reloj alcanza su `t_inicio`).

### 2. Colas (`struct Queue`)
Se implementó una estructura de cola que guarda **punteros a procesos** (`Process **procesos`), lo que permite mover procesos entre la CPU, I/O, la cola High y la cola Low moviendo solo las referencias y no copiando el `struct` completo.

---
## Flujo del Scheduler

Se tiene un ciclo `while` que avanza de a un `tick` hasta que la variable `procesos_terminados` es igual al total de procesos. Por cada `tick`, se realizan las siguientes acciones en orden:

1. **Llegadas:** se revisa el arreglo de procesos. Si el `t_inicio` de un proceso  es igual al `tick` actual, el proceso cambia a estado `READY` y se inserta al final de la cola High.
2. **Retorno de I/O:** Se revisan todos los procesos en estado `WAITING`. Se incrementa su tiempo en I/O y, si alcanzan su límite (`io_wait`), vuelven a estado `READY`, se resetea su contador y se devuelven a la cola High cumpliendo la regla del **I/O Boost**.
3. **Ascenso (Aging):** Para evitar que procesos intensivos mueran en la cola Low, se itera sobre ella. Si el proceso supera el `agingThreshold` desde la última vez que estuvo en la CPU (`t_lcpu`), se saca de Low, se resetea el quantum y se pasa a la cola High.
4. **Verificación de Deadlines:** Todo proceso en estado `READY` o `WAITING` el cual su tiempo superó el `t_deadline` se actualiza a estado `DEAD`. Se elimina de cualquier cola en la que esté y lo contamos como un proceso terminado.
5. **Métricas:** Al final de cada tick, todos los procesos que estén en estado `READY` les sumamos 1 a su contador de `waiting_time`. Falta implementar todo lo demás.

---

## Sobre el manejo de Memoria

* Se crearon las funciones `crear_queue` y `liberar_queue`.
* Todos los procesos se guardan inicialmente en un arreglo llamado `todos_los_procesos`.
* Cuando termina el `while` el programa libera los arreglos de las colas, luego las colas mismas, luego cada proceso individual, y finalmente el arreglo de todos los procesos.

---

## Faltaría implementar
Actualmente, el código tiene la base lista y falta solo hacer lo siguiente:
* **Ordenamiento EDF:** implementar `qsort` usando la función `comparación_edf`, o sea, agregar el funcionamiento dentro del while nomás.
* **Ejecución en CPU:** implementar la lógica de asignación del puntero `cpu_running`, hacer el manejo de descuentos de quantum, interrupciones por nuevos procesos en High y la ida a I/O.
* **Output CSV:** sacar las métricas del *Turnaround*, *Response Time* y *Waiting Time* para crear el archivo .csv de output.