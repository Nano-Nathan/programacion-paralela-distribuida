#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#define MAX_QUEUE 15
#define MAX_CAT 12
#define MAX_THREADS 4

//Structs
typedef struct {
    int id;
    char cat[MAX_CAT];
    float din;
} Vehicle;
typedef struct {
    bool notified;
    bool pending;
    int count;
    Vehicle data[MAX_QUEUE];
} Queue;

//Valores constantes [Moto, Coche, Utilitario, Pesado]
const float prices[] = {1, 2.5, 3.25, 5};

//Cola de vehiculos
Queue queue = {false, true, 0};
pthread_mutex_t vehicleMutex;
pthread_cond_t vehicleCond;

//Numero actual de ticket
int ticket = 0;
pthread_mutex_t ticketMutex;

//Archivo para escribir resultados
FILE *resultFILE;
pthread_mutex_t fileMutex;

//Obtiene el indice de valores para un tipo de vehiculo
float getIndex (char* value){
    if (strcmp(value, "motocicleta") == 0) {
        return 0;
    } else if (strcmp(value, "coche") == 0) {
        return 1;
    } else if (strcmp(value, "utilitario") == 0) {
        return 2;
    } else if (strcmp(value, "pesado") == 0) {
        return 3;
    }
}
//Obtiene el proximo vehiculo de la queue
Vehicle *getVehicle () {
    Vehicle *v = NULL;
    //Bloquea la mutex para obtener el vehiculo
    pthread_mutex_lock(&vehicleMutex);
    if(queue.count > 0){
        v = &queue.data[--queue.count];
        //Si hay vehiculos pendientes avisa que debe actualizar la queue si no se ha hecho
        if(queue.pending && !queue.notified && queue.count < (MAX_QUEUE - 2 * MAX_THREADS)){
            queue.notified = true;
            printf("Avisa que reponga la lista.\n");
            pthread_cond_signal(&vehicleCond);
        }
    }

    //Desbloquea el mutex
    pthread_mutex_unlock(&vehicleMutex);
    return v;
}
//Obtiene el numero de ticket
int getTicket () {
    int result = 0;
    //Bloquea la mutex para obtener el ticket
    pthread_mutex_lock(&ticketMutex);
    
    //Obtiene el numero de ticket actual
    result = ticket++;

    //Desbloquea el mutex
    pthread_mutex_unlock(&ticketMutex);

    return result;
}
//Escribe el resultado en el archivo
void writeResult (char *str) {
    //Bloquea la mutex para escribir el resultado
    pthread_mutex_lock(&fileMutex);
    
    //Escribe en el archivo
    fprintf(resultFILE, str);

    //Desbloquea el mutex
    pthread_mutex_unlock(&fileMutex);
}


void *collectorThread (void *id) {
    Vehicle *vehicle;
    int index;
    int currentTicket;
    float amountCollect;
    char strResult[50];
    printf("Ejecuta hilo %d\n", (int *)id);
    //Obtiene el vehiculo inicial
    vehicle = getVehicle();

    //Mientras haya vehiculo para cobrar
    while (vehicle != NULL) {
        //Obtiene el indice del tipo vehiculo
        index = getIndex(vehicle->cat);

        //Obtiene cuanto debe cobrarle
        amountCollect = prices[index];

        //Le cobra
        vehicle->din -= amountCollect;
        
        //Obtiene numero de ticket
        currentTicket = getTicket();

        //Espera el tiempo indicado (indice + 1)
        sleep(index + 1);

        //Escribe el resultado
        sprintf(strResult, "id:%d;cat:%s;din:%f;tick:%d\n", vehicle->id, vehicle->cat, vehicle->din, currentTicket);
        writeResult(strResult);

        printf("Hilo %d le cobra a vehiculo con id %d.\n", (int *)id, vehicle->id);

        //Obtiene el proximo vehiculo
        vehicle = getVehicle();
    }

    //Show message
    //printf("Thread collector %d\n", (int *)id);

    //Sleep random time between 0 and 1
    //sleep((double)rand() / RAND_MAX);

    //Increase variable
    //pthread_mutex_lock(&mutex);
    //compartida++;
    //pthread_mutex_unlock(&mutex);

    //End
    printf("Finaliza hilo %d\n", (int *)id);
    pthread_exit(NULL);
}

//Agrega datos a la queue
bool fillQueue (FILE *file) {
    char *token;
    int j = 0;
    printf("Agregando vehiculos a la queue..\n");
    //Mientras haya lugar en el array
    while (queue.count < MAX_QUEUE){
        char row[50];
        //Obtiene la fila a guardar
        if(fgets(row, 50, file)){
            //Splitea la cadena y el resultado es la primer key
            token = strtok(row, ";:");
            //Obtiene el value
            token = strtok(NULL, ";:");

            //Recorre la cadena spliteada
            while(token != NULL) {
                switch (j){
                    case 0: //Si j = 0, esta en el id
                        queue.data[queue.count].id = atoi(token);
                        break;
                    case 1: //Si j = 1, esta en la cat
                        strcpy(queue.data[queue.count].cat, token);
                        break;
                    case 2: //Si j = 2, esta en din
                        //Reemplazo la , por un . para no tener problemas en el mapeo
                        for (int k = 0; k < strlen(token); k++) {
                            if (token[k] == ',') {
                                token[k] = '.';
                                break;
                            }
                        }
                        queue.data[queue.count].din = atof(token);
                        //sscanf(token, "%f", &queue->data[queue->count].din);
                        //Avisa que actualice el siguiente item
                        queue.count++;
                        //Reinicia el contador que avisa el valor a actualizar
                        j = -1;
                        break;
                }
                //Pasa al siguiente value
                j++;
                //Obtiene la key
                token = strtok(NULL, ";:");
                //Obtiene el value
                token = strtok(NULL, ";:");
            }
        } else {
            queue.pending = false;
            queue.notified = true;
            return false;
        }
    }
    
    printf("Se han agregado los vehiculos.\n");
    queue.pending = true;
    queue.notified = false;
    return true;
}
void *fillerThread (void *id) {
    bool thereVehicles = true;
    printf("Ejecuta hilo que repone la queue\n");
    //Abre el archivo
    FILE* file = fopen("autopista.in", "r");
    if (file == NULL) {
        printf("Error al abrir el archivo.\n");
    } else {
        //Bloquea el mutex para inicializar
        pthread_mutex_lock(&vehicleMutex);

        //Inicializa la cola y obtiene si hay mas vehiculos en espera
        thereVehicles = fillQueue(file);

        //Desbloquea el mutex
        pthread_mutex_unlock(&vehicleMutex);

        //Mientras haya vehiculos para ingresar
        while (thereVehicles){
            //Bloquea el mutex para llenar
            pthread_mutex_lock(&vehicleMutex);
            //Espera la señal de agregacion
            pthread_cond_wait(&vehicleCond, &vehicleMutex);
            
            //Ingresa vehiculos y obtiene si hay mas en espera
            thereVehicles = fillQueue(file);

            //Desbloquea el mutex
            pthread_mutex_unlock(&vehicleMutex);
        }
        //Close file
        fclose(file);
    }
    printf("No hay mas vehiculos para reponer. Fin thread\n");
    //End
    pthread_exit(NULL);
}

void initShares () {
    pthread_mutex_init(&fileMutex, NULL);
    pthread_mutex_init(&ticketMutex, NULL);
    pthread_mutex_init(&vehicleMutex, NULL);
    pthread_cond_init(&vehicleCond, NULL);
    //for (int i = 0; i < MAX_THREADS + 1; i++){
    //    pthread_mutex_init(&mutex[i], NULL);
    //    pthread_cond_init(&cond[i], NULL);
    //}

}
void destroyShares () {
    pthread_mutex_destroy(&fileMutex);
    pthread_mutex_destroy(&ticketMutex);
    pthread_mutex_destroy(&vehicleMutex);
    pthread_cond_destroy(&vehicleCond);
//    for (int i = 0; i < MAX_THREADS + 1; i++){
//        pthread_mutex_destroy(&mutex[i]);
//        pthread_cond_destroy(&cond[i]);
//    }

}

int main(int count, char *parameters[]) {
    //Abre el archivo
    resultFILE = fopen("result.out", "w");
    if (resultFILE == NULL) {
        printf("Error al abrir el archivo de resultados.\n");
    } else {
        //Array de threads
        pthread_t aThreads[MAX_THREADS + 1];

        //Inicializa mutexs y condicionales
        initShares();

        //Crea el thread que llenara la cola
        pthread_t t;
        if (pthread_create(&t, NULL, fillerThread, NULL) != 0){
            printf("No se ha podido crear el thread llenador.\n");
        } else {
            aThreads[0] = t;
        }

        //Crea los threads que atenderan los puestos
        for (int i = 1; i < MAX_THREADS + 1; i++){
            pthread_t t;
            if (pthread_create(&t, NULL, collectorThread, (void *)i) != 0){
                printf("No se ha podido crear el thread recaudador %d.\n", i);
            } else {
                aThreads[i] = t;
            }
        }


        //Espera la ejecucion de los threads primeramente al que rellena la cola
        for (int i = 0; i < MAX_THREADS + 1; i++){
            pthread_join(aThreads[i], NULL);
        }

        //Destruye las mutexs y condicionales
        destroyShares();
        //Cierra el archivo de resultados
        fclose(resultFILE);
    }
    printf("Fin del programa.\n");
}