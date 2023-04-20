#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

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
    int count;
    Vehicle data[MAX_QUEUE];
} Queue;


//Numero actual de ticket
int ticket = 0;
pthread_mutex_t ticketMutex;

//Contador que muestra cuantos hilos deben terminar su ejecucion
int finish = (2 * MAX_THREADS) + 1;
pthread_mutex_t finishMutex;
pthread_cond_t finishCond;

//Mutex para coordinar la cola de vehiculos. 0: llenando, 1: hay vehiculos, -1: no hay mas pendientes
int statusQueue = 0;
pthread_mutex_t vehicleMutex;
pthread_cond_t vehicleCond;
pthread_mutex_t vehicleFillMutex;
pthread_cond_t vehicleFillCond;

//Condicionales y sus mutexs. de 0 a 4 para threads y asistentes y la 5ta para rellenar la cola
pthread_cond_t cond[MAX_THREADS + 1];
pthread_mutex_t mutex[MAX_THREADS + 1];

//Metodo para obtener el precio de un vehiculo
float getPrice (char* value){
    if (strcmp(value, "motocicleta") == 0) {
        return 1;
    } else if (strcmp(value, "coche") == 0) {
        return 2.5;
    } else if (strcmp(value, "utilitario") == 0) {
        return 3.25;
    } else if (strcmp(value, "pesado") == 0) {
        return 5;
    }
}
//Para notificar que un hilo termina
void notifyFinish (char a[]) {
    pthread_mutex_lock(&finishMutex);
    finish--;
    printf("Notifica fin %s %d\n", a, finish);
    pthread_cond_signal(&finishCond);
    pthread_mutex_unlock(&finishMutex);
}


void *collectorThread (void *id) {
    //Show message
    //printf("Thread collector %d\n", (int *)id);

    //Sleep random time between 0 and 1
    //sleep((double)rand() / RAND_MAX);

    //Increase variable
    //pthread_mutex_lock(&mutex);
    //compartida++;
    //pthread_mutex_unlock(&mutex);

    //End
    notifyFinish(" collector ");
    pthread_exit(NULL);
}
void *wizardThread (void *id) {
    int i = (int)id;
    //Show message
    //printf("Thread wizard %d\n", (int *)id);

    //Sleep random time between 0 and 1

    //Increase variable
    //pthread_mutex_lock(&mutex);
    //compartida++;
    //pthread_mutex_unlock(&mutex);

    //End
    notifyFinish(" wizard ");
    pthread_exit(NULL);
}

Vehicle getVehicle (Queue *queue) {
    if(queue->count > 0){
        return queue->data[--queue->count];
    }
}
bool updateQueue (FILE *file, Queue *queue) {
    char *token;
    int j = 0;

    //Mientras haya lugar en el array
    while (queue->count < MAX_QUEUE){
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
                        queue->data[queue->count].id = atoi(token);
                        break;
                    case 1: //Si j = 1, esta en la cat
                        strcpy(queue->data[queue->count].cat, token);
                        break;
                    case 2: //Si j = 2, esta en din
                        //Reemplazo la , por un . para no tener problemas en el mapeo
                        for (int k = 0; k < strlen(token); k++) {
                            if (token[k] == ',') {
                                token[k] = '.';
                                break;
                            }
                        }
                        queue->data[queue->count].din = atof(token);
                        //sscanf(token, "%f", &queue->data[queue->count].din);
                        //Avisa que actualice el siguiente item
                        queue->count++;
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
            printf("No hay mas vehiculos.\n");
            return false;
        }
    }
    return true;
}
bool fillQueue (FILE *file, Queue *queue) {
    bool result;
    //Bloquea el mutex
    pthread_mutex_lock(&vehicleMutex);
    printf("Llena queue\n");
    //Llena la cola
    result = updateQueue(file, queue);
    //Notifica si hay vehiculos
    statusQueue = result ? 1 : -1;
    pthread_cond_broadcast(&vehicleCond);
    //Desbloquea el mutex
    pthread_mutex_unlock(&vehicleMutex);
}
void *fillerThread (void *id) {
    bool thereVehicles = true;
    //Cola de vehiculos
    Queue queue = {0, MAX_QUEUE};

    //Abre el archivo
    FILE* file = fopen("autopista.in", "r");
    if (file == NULL) {
        printf("Error al abrir el archivo.\n");
        //Avisa que no hay vehiculos
        pthread_mutex_lock(&vehicleMutex);
        statusQueue = -1;
        pthread_cond_signal(&vehicleCond);
        pthread_mutex_unlock(&vehicleMutex);
    } else {
        //Inicializa la cola
        thereVehicles = fillQueue(file, &queue);

        //Mientras haya vehiculos para ingresar
        while (thereVehicles){
            printf("Espera llenado\n");
            //Bloquea el mutex para llenar
            pthread_mutex_lock(&vehicleFillMutex);
            //Espera la señal
            pthread_cond_wait(&vehicleFillCond, &vehicleFillMutex);
            
            //Ingresa vehiculos
            thereVehicles = fillQueue(file, &queue);

            //Desbloquea el mutex
            pthread_mutex_unlock(&vehicleFillMutex);
        }

        //Close file
        fclose(file);
    }
    //End
    notifyFinish(" filler ");
    pthread_exit(NULL);
}

void initShares () {
    pthread_mutex_init(&ticketMutex, NULL);
    pthread_mutex_init(&vehicleMutex, NULL);
    pthread_cond_init(&vehicleCond, NULL);
    pthread_mutex_init(&vehicleFillMutex, NULL);
    pthread_cond_init(&vehicleFillCond, NULL);
    pthread_mutex_init(&finishMutex, NULL);
    pthread_cond_init(&finishCond, NULL);
    for (int i = 0; i < MAX_THREADS + 1; i++){
        pthread_mutex_init(&mutex[i], NULL);
        pthread_cond_init(&cond[i], NULL);
    }
    
}
void destroyShares () {
    pthread_mutex_destroy(&ticketMutex);
    pthread_mutex_destroy(&vehicleMutex);
    pthread_cond_destroy(&vehicleCond);
    pthread_mutex_destroy(&vehicleFillMutex);
    pthread_cond_destroy(&vehicleFillCond);
    pthread_mutex_destroy(&finishMutex);
    pthread_cond_destroy(&finishCond);
    for (int i = 0; i < MAX_THREADS + 1; i++){
        pthread_mutex_destroy(&mutex[i]);
        pthread_cond_destroy(&cond[i]);
    }
}

int main(int count, char *parameters[]) {
    //Inicializa mutexs y condicionales
    initShares();

    //Crea los threads que atenderan las respuestas de los puestos
    for (int i = 0; i < MAX_THREADS; i++){
        pthread_t t;
        if (pthread_create(&t, NULL, wizardThread, (void *)i) != 0){
            printf("No se ha podido crear el thread asistente %d.\n", i);
        }
    }

    //Crea los threads que atenderan los puestos
    for (int i = 0; i < MAX_THREADS; i++){
        pthread_t t;
        if (pthread_create(&t, NULL, collectorThread, (void *)i) != 0){
            printf("No se ha podido crear el thread recaudador %d.\n", i);
        }
    }

    //Crea el thread que llenara la cola
    pthread_t t;
    if (pthread_create(&t, NULL, fillerThread, NULL) != 0){
        printf("No se ha podido crear el thread llenador.\n");
    }

    //Mientras haya threads ejecutando
    while (finish > 0){
        printf("Espera fin\n");
        //Bloquea el mutex para finalizar
        pthread_mutex_lock(&finishMutex);
        //Espera la señal
        pthread_cond_wait(&finishCond, &finishMutex);
        //Desbloquea el mutex
        pthread_mutex_unlock(&finishMutex);
    }

    //Destruye las mutexs y condicionales
    destroyShares();
    printf("Fin del programa.\n");
}