#include "includes.h"
#include "global.h"
#include "fonctions.h"

/**
 * \fn void initStruct(void)
 * \brief Initialisation des structures de l'application (tâches, mutex, 
 * semaphore, etc.)
 */
void initStruct(void);

/**
 * \fn void startTasks(void)
 * \brief Démarrage des tâches
 */
void startTasks(void);

/**
 * \fn void deleteTasks(void)
 * \brief Arrêt des tâches
 */
void deleteTasks(void);

int main(int argc, char**argv) {
    printf("#################################\n");
    printf("#      DE STIJL PROJECT         #\n");
    printf("#################################\n");

    //signal(SIGTERM, catch_signal);
    //signal(SIGINT, catch_signal);

    /* Avoids memory swapping for this program */
    mlockall(MCL_CURRENT | MCL_FUTURE);
    /* For printing, please use rt_print_auto_init() and rt_printf () in rtdk.h
     * (The Real-Time printing library). rt_printf() is the same as printf()
     * except that it does not leave the primary mode, meaning that it is a
     * cheaper, faster version of printf() that avoids the use of system calls
     * and locks, instead using a local ring buffer per real-time thread along
     * with a process-based non-RT thread that periodically forwards the
     * contents to the output stream. main() must call rt_print_auto_init(1)
     * before any calls to rt_printf(). If you forget this part, you won't see
     * anything printed.
     */
    rt_print_auto_init(1);
    initStruct();
    startTasks();
    pause();
    deleteTasks();

    return 0;
}

void initStruct(void) {
    int err;
    /* Creation des mutex */
    if ((err = rt_mutex_create(&mutexEtatComMoniteur, NULL))) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_mutex_create(&mutexEtatComRobot, NULL))) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_mutex_create(&mutexMove, NULL))) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_mutex_create(&mutexActionArena, NULL))) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_mutex_create(&mutexPositionRobot, NULL))) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_mutex_create(&mutexContinu, NULL))) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_mutex_create(&mutexCheck_etatComRobot, NULL))) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_mutex_create(&mutexServeur, NULL))) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_mutex_create(&mutexRobot, NULL))) {
        rt_printf("Error mutex create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    
    /* Creation du semaphore */
    if ((err = rt_sem_create(&semConnecterRobot, NULL, 0, S_FIFO))) {
        rt_printf("Error semaphore create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_sem_create(&semStartWatchdog, NULL, 0, S_FIFO))) {
        rt_printf("Error semaphore create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    /* Creation des taches */
    if ((err = rt_task_create(&tRecevoirMoniteur, NULL, 0, PRIORITY_TRECEVOIR_MONITEUR, 0))) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_task_create(&tConnect, NULL, 0, PRIORITY_TCONNECT, 0))) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_task_create(&tMove, NULL, 0, PRIORITY_TMOVE, 0))) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_task_create(&tEnvoyer, NULL, 0, PRIORITY_TENVOYER, 0))) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_task_create(&tWatchdog, NULL, 0, PRIORITY_TWATCHDOG, 0))) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_task_create(&tTraitement_img, NULL, 0, PRIORITY_TTRAITEMENT_IMG, 0))) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_task_create(&tEtat_bat, NULL, 0, PRIORITY_TETAT_BAT, 0))) {
        rt_printf("Error task create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }


    /* Creation des files de messages */
    if ((err = rt_queue_create(&queueMsgGUI, "toto", MSG_QUEUE_SIZE * sizeof (DMessage), MSG_QUEUE_SIZE, Q_FIFO))) {
        rt_printf("Error msg queue create: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

    /* Creation des structures globales du projet */
    robot = d_new_robot();
    move = d_new_movement();
    serveur = d_new_server();
    positionRobot = d_new_position();
    actionArena = d_new_action();
    
    /* Initialisation des variables globales du projet */
    actionArena->set_order(actionArena, NO_ACTION_ARENA);
    etatComMoniteur = STATUS_ERR_NO_FILE;
    etatComRobot = STATUS_ERR_NO_FILE;
}

void startTasks() {
    int err;
    if ((err = rt_task_start(&tConnect, &connecter, NULL))) {
        rt_printf("Error task start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_task_start(&tRecevoirMoniteur, &recevoir_moniteur, NULL))) {
        rt_printf("Error task start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_task_start(&tMove, &deplacer, NULL))) {
        rt_printf("Error task start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_task_start(&tEnvoyer, &envoyer_moniteur, NULL))) {
        rt_printf("Error task start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_task_start(&tWatchdog, &watchdog, NULL))) {
        rt_printf("Error task start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_task_start(&tTraitement_img, &traitement_image, NULL))) {
        rt_printf("Error task start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }
    if ((err = rt_task_start(&tEtat_bat, &etat_batterie, NULL))) {
        rt_printf("Error task start: %s\n", strerror(-err));
        exit(EXIT_FAILURE);
    }

}

void deleteTasks() {
    rt_task_delete(&tRecevoirMoniteur);
    rt_task_delete(&tConnect);
    rt_task_delete(&tMove);
    rt_task_delete(&tEnvoyer);
    rt_task_delete(&tWatchdog);
    rt_task_delete(&tTraitement_img);
    rt_task_delete(&tEtat_bat);
}
