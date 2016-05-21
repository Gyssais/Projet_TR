#include "fonctions.h"

#define DEBUG 0
#define IF_DEBUG if(DEBUG)

#define ENABLE_T_ENVOYER_MONITEUR 1
#define ENABLE_T_RECEVOIR_MONITEUR 1
#define ENABLE_T_CONNECTER 1
#define ENABLE_T_DEPLACER 1
#define ENABLE_T_WATCHDOG 1
#define ENABLE_T_TRAITEMENT_IMAGE 1
#define ENABLE_T_ETAT_BATTERIE 1

#define CAMERA_NUMBER 0
#define SEUIL_VITESSE_RAPIDE 50

#define INSECURE 0
#define COM_ROBOT_NB_ESSAIS 50


int write_in_queue(RT_QUEUE *msgQueue, void * data, int size);
int check_etatComRobot(int status, int connect);

void envoyer_moniteur(void * arg) {
#if ENABLE_T_ENVOYER_MONITEUR
    DMessage *msg;
    int err;

    IF_DEBUG rt_printf("t_envoyer_moniteur : Début de l'exécution\n");
    while (1) {
        printf("A\n");
        rt_printf("t_envoyer_moniteur : Attente d'un message...\n");
        if ((err = rt_queue_read(&queueMsgGUI, &msg, sizeof (DMessage), TM_INFINITE)) >= 0) {
            IF_DEBUG rt_printf("t_envoyer_moniteur : Envoi d'un message au moniteur...\n");
            printf("B\n");
            //rt_mutex_acquire(&mutexServeur, TM_INFINITE);
            if (msg == NULL) {printf("message NULL");}
            if (serveur->send(serveur, msg) < 0)
                IF_DEBUG rt_printf("t_envoyer_moniteur : Echec envoi d'un message au moniteur !\n");
            else
                IF_DEBUG rt_printf("t_envoyer_moniteur : Message au moniteur envoyé\n");
            //rt_mutex_release(&mutexServeur);
            printf("C\n");
            msg->free(msg);
            printf("D\n");
        } else {
            rt_printf("t_envoyer_moniteur : Error msg queue read: %s\n", strerror(-err));
        }
    }
#endif
}

void recevoir_moniteur(void *arg) {
#if ENABLE_T_RECEVOIR_MONITEUR
    DMessage *msg = d_new_message();
    DAction *action = d_new_action();
    int size = 0;
    int num_msg = 0;
    int order = 0;

    IF_DEBUG rt_printf("t_recevoir_moniteur : Début de l'exécution\n");

    while (1) {
        rt_mutex_acquire(&mutexEtatComMoniteur, TM_INFINITE);
        etatComMoniteur = STATUS_ERR_NO_FILE;
        rt_mutex_release(&mutexEtatComMoniteur);

        rt_printf("t_recevoir_moniteur : Attente de demande de connexion du moniteur...\n");
        //rt_mutex_acquire(&mutexServeur, TM_INFINITE);
        serveur->open(serveur, "8000");
        //rt_mutex_release(&mutexServeur);
        rt_printf("t_recevoir_moniteur : Connecté au moniteur\n");

        rt_mutex_acquire(&mutexEtatComMoniteur, TM_INFINITE);
        etatComMoniteur = STATUS_OK;
        rt_mutex_release(&mutexEtatComMoniteur);

        do {
            rt_printf("t_recevoir_moniteur : Attente d'un message\n");
            // rt_mutex_acquire(&mutexServeur, TM_INFINITE);
            size = serveur->receive(serveur, msg);
            // rt_mutex_release(&mutexServeur);
            num_msg++;
            if (size > 0) {
                switch (msg->get_type(msg)) {
                    case MESSAGE_TYPE_ACTION:
                        rt_printf("t_recevoir_moniteur : Le message %d reçu est une action\n", num_msg);
                        action->from_message(action, msg);
                        order = action->get_order(action);
                        switch (order) {
                            case ACTION_CONNECT_ROBOT:
                                rt_printf("t_recevoir_moniteur : Action connecter robot\n");
                                rt_sem_v(&semConnecterRobot);
                                break;
                            case ACTION_FIND_ARENA:
                            case ACTION_ARENA_IS_FOUND:
                            case ACTION_ARENA_FAILED:
                                rt_printf("t_recevoir_moniteur : Action arène, ordre %d\n", order);
                                rt_mutex_acquire(&mutexActionArena, TM_INFINITE);
                                actionArena->set_order(actionArena, order);
                                rt_mutex_release(&mutexActionArena);
                                break;
                            case ACTION_COMPUTE_CONTINUOUSLY_POSITION:
                                rt_printf("t_recevoir_moniteur : Action calcul continu ON\n");
                                rt_mutex_acquire(&mutexContinu, TM_INFINITE);
                                continu = 1;
                                rt_mutex_release(&mutexContinu);
                                break;
                            case ACTION_STOP_COMPUTE_POSITION:
                                rt_printf("t_recevoir_moniteur : Action calcul continu OFF\n");
                                rt_mutex_acquire(&mutexContinu, TM_INFINITE);
                                continu = 0;
                                rt_mutex_release(&mutexContinu);
                                break;
                        }
                        break;
                    case MESSAGE_TYPE_MOVEMENT:
                        rt_printf("t_recevoir_moniteur : Le message reçu %d est un mouvement\n", num_msg);
                        rt_mutex_acquire(&mutexMove, TM_INFINITE);
                        move->from_message(move, msg);
                        rt_mutex_release(&mutexMove);
                        break;
                }
            }
        } while (size > 0);

        rt_printf("t_recevoir_moniteur : Connexion au moniteur perdue !\n");
        // rt_mutex_acquire(&mutexServeur, TM_INFINITE);
        d_server_close(serveur);
        // rt_mutex_release(&mutexServeur);
    }
    // msg->free(msg);
#endif
}

void connecter(void * arg) {
#if ENABLE_T_CONNECTER
    int status;
    int statusRobot;
    DMessage *message;

    IF_DEBUG rt_printf("t_connect : Début de l'exécution\n");

    while (1) {
        rt_printf("t_connect : Attente du sémarphore semConnecterRobot...\n");
        rt_sem_p(&semConnecterRobot, TM_INFINITE);

        rt_printf("t_connect : Ouverture de la communication avec le robot...\n");
        do {
            rt_mutex_acquire(&mutexRobot, TM_INFINITE);
            status = d_robot_open_device(robot);
            rt_mutex_release(&mutexRobot);

            // IF_DEBUG rt_printf("t_connect : status open device : %d\n", status);

            // rt_mutex_acquire(&mutexCheck_etatComRobot, TM_INFINITE);
            statusRobot = check_etatComRobot(status, 1);
            // rt_mutex_release(&mutexCheck_etatComRobot);

        } while (status != STATUS_OK && statusRobot == STATUS_OK);

        if (statusRobot != STATUS_OK) {
            IF_DEBUG rt_printf("t_connect : Echec de la connexion au robot !\n");
            continue;
        }

        rt_printf("t_connect : Connexion avec le robot établie\n");
        rt_mutex_acquire(&mutexEtatComRobot, TM_INFINITE);
        etatComRobot = status;
        rt_mutex_release(&mutexEtatComRobot);

        if (INSECURE) rt_printf("t_connect : Mode Insecure !\n");
        rt_printf("t_connect : Démarrage du robot...\n");
        do {
            rt_mutex_acquire(&mutexRobot, TM_INFINITE);
            if (INSECURE) status = d_robot_start_insecurely(robot);
            else status = d_robot_start(robot);
            rt_mutex_release(&mutexRobot);

            // IF_DEBUG rt_printf("t_connect : status start : %d\n", status);

            // rt_mutex_acquire(&mutexCheck_etatComRobot, TM_INFINITE);
            statusRobot = check_etatComRobot(status, 0);
            // rt_mutex_release(&mutexCheck_etatComRobot);

        } while (status != STATUS_OK && statusRobot == STATUS_OK);

        if (statusRobot != STATUS_OK) {
            rt_printf("t_connect : Echec du démarrage du robot !\nVérifier que le robot est allumé.\n");
            continue;
        }

        rt_printf("t_connect : Robot démarré\n");

        message = d_new_message();
        message->put_state(message, statusRobot);

        rt_printf("t_connect : Envoi message status...\n");
        if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
            message->free(message);
            IF_DEBUG rt_printf("t_connect :  Echec envoi message status !\n");
        } else IF_DEBUG rt_printf("t_connect : Message status envoyé\n");

        rt_printf("t_connect : Ordre de lancement du watchdog\n");
        rt_sem_v(&semStartWatchdog);
    }
    // message->free(message);
#endif
}

void deplacer(void *arg) {
#if ENABLE_T_DEPLACER
    int status;
    int statusRobot;
    int gauche;
    int droite;
    int direction;
    //int vitesse;

    IF_DEBUG rt_printf("t_deplacer : Début de l'éxecution periodique à 1s\n");
    rt_task_set_periodic(NULL, TM_NOW, 1e+09);

    while (1) {
        //Attente de l'activation périodique 
        rt_task_wait_period(NULL);
        IF_DEBUG rt_printf("t_deplacer : Activation périodique\n");

        rt_mutex_acquire(&mutexEtatComRobot, TM_INFINITE);
        status = etatComRobot;
        rt_mutex_release(&mutexEtatComRobot);

        if (status == STATUS_OK) {

            rt_mutex_acquire(&mutexMove, TM_INFINITE);
            direction = move->get_direction(move);
            //vitesse = move->get_speed(move);
            rt_mutex_release(&mutexMove);
            switch (direction) {
                case DIRECTION_FORWARD:
                    gauche = MOTEUR_ARRIERE_LENT;
                    droite = MOTEUR_ARRIERE_LENT;
                    break;
                case DIRECTION_LEFT:
                    gauche = MOTEUR_ARRIERE_LENT;
                    droite = MOTEUR_AVANT_LENT;
                    break;
                case DIRECTION_RIGHT:
                    gauche = MOTEUR_AVANT_LENT;
                    droite = MOTEUR_ARRIERE_LENT;
                    break;
                case DIRECTION_STOP:
                    gauche = MOTEUR_STOP;
                    droite = MOTEUR_STOP;
                    break;
                case DIRECTION_STRAIGHT:
                    gauche = MOTEUR_AVANT_LENT;
                    droite = MOTEUR_AVANT_LENT;
                    break;
            }
            /*rt_printf("vitesse:%d\n");
            if (vitesse > SEUIL_VITESSE_RAPIDE) {
                gauche *= 2;
                droite *= 2;
            }*/

            IF_DEBUG rt_printf("t_deplacer : Commande des moteurs...\n");
            do {
                rt_mutex_acquire(&mutexRobot, TM_INFINITE);
                status = robot->set_motors(robot, gauche, droite);
                rt_mutex_release(&mutexRobot);

                // rt_mutex_acquire(&mutexCheck_etatComRobot, TM_INFINITE);
                statusRobot = check_etatComRobot(status, 0);
                // rt_mutex_release(&mutexCheck_etatComRobot);
            } while (status != STATUS_OK && statusRobot == STATUS_OK);

            if (status == STATUS_OK) {
                IF_DEBUG rt_printf("t_deplacer : Commande des moteurs réussie\n");
            } else IF_DEBUG rt_printf("t_deplacer : Echec de la commande des moteurs !\n");

        } else IF_DEBUG rt_printf("t_deplacer : Robot non connecté !\n");
    }
#endif
}

void etat_batterie(void *arg) {
#if ENABLE_T_ETAT_BATTERIE
    int status;
    int statusRobot;
    int vbat;
    DBattery * batterie = d_new_battery();
    DMessage *message;

    IF_DEBUG rt_printf("t_etat_batterie : Début de l'éxecution periodique à 250ms\n");
    rt_task_set_periodic(NULL, TM_NOW, 250e+06);

    while (1) {
        // Attente de l'activation périodique
        rt_task_wait_period(NULL);
        IF_DEBUG rt_printf("t_etat_batterie : Activation périodique\n");

        rt_mutex_acquire(&mutexEtatComRobot, TM_INFINITE);
        status = etatComRobot;
        rt_mutex_release(&mutexEtatComRobot);

        if (status == STATUS_OK) {
            do {
                rt_mutex_acquire(&mutexRobot, TM_INFINITE);
                status = d_robot_get_vbat(robot, &vbat);
                rt_mutex_release(&mutexRobot);

                // rt_mutex_acquire(&mutexCheck_etatComRobot, TM_INFINITE);
                statusRobot = check_etatComRobot(status, 0);
                // rt_mutex_release(&mutexCheck_etatComRobot);

            } while (status != STATUS_OK && statusRobot == STATUS_OK);

            if (statusRobot == STATUS_OK) {
                batterie ->set_level(batterie, vbat);

                message = d_new_message();
                message ->put_battery_level(message, batterie);

                IF_DEBUG rt_printf("t_etat_batterie : Envoi message batterie...\n");
                if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
                    message->free(message);
                    IF_DEBUG rt_printf("t_etat_batterie : Echec envoi message batterie !\n");
                } else IF_DEBUG rt_printf("t_connect : Message batterie envoyé\n");
            }
        } else IF_DEBUG rt_printf("t_etat_batterie : Robot non connecté !\n");
    }
    // message->free(message);
    // batterie->free(batterie);
#endif
}

void watchdog(void *arg) {
#if ENABLE_T_WATCHDOG
    int statusRobot;
    int status;

    IF_DEBUG rt_printf("t_recevoir_moniteur : Début de l'exécution\n");
    rt_task_set_periodic(NULL, TM_NOW, 1e+09);

    while (1) {
        rt_printf("t_watchdog : Attente du sémarphore semStartWatchdog...\n");
        rt_sem_p(&semStartWatchdog, TM_INFINITE);
        rt_printf("t_watchdog : Début de l'éxecution periodique à 1s\n");

        do {
            // Attente de l'activation périodique 
            rt_task_wait_period(NULL);
            rt_printf("t_watchdog : Activation périodique\n");

            rt_mutex_acquire(&mutexEtatComRobot, TM_INFINITE);
            status = etatComRobot;
            rt_mutex_release(&mutexEtatComRobot);

            if (status != STATUS_OK) break;

            do {
                rt_mutex_acquire(&mutexRobot, TM_INFINITE);
                status = d_robot_reload_wdt(robot);
                rt_mutex_release(&mutexRobot);

                // rt_mutex_acquire(&mutexCheck_etatComRobot, TM_INFINITE);
                statusRobot = check_etatComRobot(status, 0);
                // rt_mutex_release(&mutexCheck_etatComRobot);

            } while (status != STATUS_OK && statusRobot == STATUS_OK);

        } while (statusRobot == STATUS_OK);
    }
#endif
}

void traitement_image(void *arg) {
#if ENABLE_T_TRAITEMENT_IMAGE
    int status;
    DCamera * camera = d_new_camera();
    DImage * img = d_new_image();
    int etatComCamera = STATUS_ERR_NO_FILE;
    int order;
    DArena * arena;
    DArena * savedArena = NULL;
    DPosition * position;
    DMessage * message_image;
    DMessage * message_position;
    DJpegimage * jpegImg = d_new_jpegimage();
    int cont;

    IF_DEBUG rt_printf("t_traitement_image : Début de l'éxecution periodique à 600ms\n");
    rt_task_set_periodic(NULL, TM_NOW, 600e+6);

    while (1) {
        printf("0\n");
        rt_task_wait_period(NULL);
        IF_DEBUG rt_printf("t_traitement_image : Activation périodique\n");
        printf("1\n");


        rt_mutex_acquire(&mutexEtatComMoniteur, TM_INFINITE);
        status = etatComMoniteur;
        rt_mutex_release(&mutexEtatComMoniteur);

        if (status == STATUS_OK) {
            etatComCamera = (camera->mIndice >= 0) ? STATUS_OK : STATUS_ERR_NO_FILE;
            if (etatComCamera != STATUS_OK) {
                IF_DEBUG rt_printf("t_traitement_image : Connexion à la caméra...\n");
                d_camera_open_number(camera, CAMERA_NUMBER);
                if (camera->mIndice >= 0) {
                    etatComCamera = STATUS_OK;
                    IF_DEBUG rt_printf("t_traitement_image : Caméra connectée\n");
                } else {
                    etatComCamera = STATUS_ERR_NO_FILE;
                    IF_DEBUG rt_printf("t_traitement_image : Echec de connexion à la caméra !\n");
                }
            }
            if (etatComCamera == STATUS_OK) {

                d_camera_get_frame(camera, img);
                rt_mutex_acquire(&mutexActionArena, TM_INFINITE);
                order = actionArena->get_order(actionArena);
                rt_mutex_release(&mutexActionArena);

                switch (order) {
                    case ACTION_FIND_ARENA:
                        IF_DEBUG rt_printf("t_traitement_image : Trouver arène\n");
                        arena = d_image_compute_arena_position(img);
                        d_imageshop_draw_arena(img, arena);
                        break;

                    case ACTION_ARENA_IS_FOUND:
                        IF_DEBUG rt_printf("t_traitement_image : Arène trouvée\n");
                        d_arena_free(savedArena);
                        savedArena = arena;
                        arena = d_new_arena();

                    case ACTION_ARENA_FAILED:
                        rt_mutex_acquire(&mutexActionArena, TM_INFINITE);
                        actionArena->set_order(actionArena, NO_ACTION_ARENA);
                        rt_mutex_release(&mutexActionArena);
                        break;

                    case NO_ACTION_ARENA:
                        IF_DEBUG rt_printf("t_traitement_image : Pas d'action d'arène\n");
                        rt_mutex_acquire(&mutexContinu, TM_INFINITE);
                        cont = continu;
                        rt_mutex_release(&mutexContinu);
                        if (cont) {
                            IF_DEBUG rt_printf("t_traitement_image : Calcul continu\n");
                            // Debug
                            if (img == NULL) printf("img NULL\n");
                            if(savedArena == NULL) printf("savedArena NULL\n");
                            //
                            position = d_image_compute_robot_position(img, savedArena);
                            if (position == NULL) {printf("Erreur calcul position !\n");} else {printf("Position calculée\n");}
                            printf("5\n");
                            rt_mutex_acquire(&mutexPositionRobot, TM_INFINITE);
                            d_position_free(positionRobot);/**/
                            positionRobot = position;
                            rt_mutex_release(&mutexPositionRobot);
                            
                            d_imageshop_draw_arena(img, savedArena);
                            d_imageshop_draw_position(img, positionRobot);
                            if (position != NULL) {
                                message_position = d_new_message();
                                message_position->put_position(message_position, position);
                                IF_DEBUG rt_printf("t_traitement_image : Envoi message position...\n");
                                if (write_in_queue(&queueMsgGUI, message_position, sizeof (DMessage)) < 0) {
                                    message_position->free(message_position);
                                    IF_DEBUG rt_printf("t_traitement_image : Echec envoi message position !\n");
                                } else IF_DEBUG rt_printf("t_traitement_image : Message position envoyé\n");
                                d_position_free(position);/**/
                            }
                        }
                }
                d_jpegimage_compress(jpegImg, img);
                printf("9\n"); // Ce printf() évite une erreur de segmentation : pourquoi ?
                
                message_image = d_new_message();
                message_image->put_jpeg_image(message_image, jpegImg);
                IF_DEBUG rt_printf("t_traitement_image : Envoi message image...\n");
                if (write_in_queue(&queueMsgGUI, message_image, sizeof (DMessage)) < 0) {
                    message_image->free(message_image);
                    IF_DEBUG rt_printf("t_traitement_image : Echec envoi message image !\n");
                } else IF_DEBUG rt_printf("t_traitement_image : Message image envoyé\n");
                
                
            } else IF_DEBUG rt_printf("t_traitement_image : Caméra non connectée !\n");
        } else IF_DEBUG rt_printf("t_traitement_image : Moniteur non connecté !\n");
    }
    // message->free(message);
#endif
}

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size) {
    void *msg;
    int err;

    msg = rt_queue_alloc(msgQueue, size);
    memcpy(msg, &data, size);

    if ((err = rt_queue_send(msgQueue, msg, sizeof (DMessage), Q_NORMAL)) < 0) {
        rt_printf("Error msg queue send: %s\n", strerror(-err));
    }
    rt_queue_free(&queueMsgGUI, msg);

    return err;
}

int check_etatComRobot(int status, int connect) {
    int statusRobot = status;
    static int cpt = 0;
    DMessage * message;

    if (!connect) {
        rt_mutex_acquire(&mutexEtatComRobot, TM_INFINITE);
        statusRobot = etatComRobot;
        rt_mutex_release(&mutexEtatComRobot);
    }
    if ((statusRobot == STATUS_OK && !connect) || connect) {
        if (status == STATUS_OK) {
            rt_mutex_acquire(&mutexCheck_etatComRobot, TM_INFINITE);
            cpt = 0;
            rt_mutex_release(&mutexCheck_etatComRobot);
        } else {
            rt_mutex_acquire(&mutexCheck_etatComRobot, TM_INFINITE);
            cpt++;
            if (cpt > COM_ROBOT_NB_ESSAIS) {
                cpt = 0;
                rt_mutex_release(&mutexCheck_etatComRobot);

                rt_printf("t_check_etatComRobot : Connexion avec le robot perdue !\n");

                rt_printf("t_check_etatComRobot : Fermeture de connexion...\n");
                rt_mutex_acquire(&mutexRobot, TM_INFINITE);
                if (d_robot_close_com(robot) != STATUS_OK)
                    rt_printf("t_check_etatComRobot : Echec fermeture de connexion !\n");
                rt_mutex_release(&mutexRobot);
                rt_printf("t_check_etatComRobot : Connexion fermée\n");

                rt_mutex_acquire(&mutexEtatComRobot, TM_INFINITE);
                etatComRobot = status;
                rt_mutex_release(&mutexEtatComRobot);

                message = d_new_message();
                message->put_state(message, status);

                IF_DEBUG rt_printf("t_check_etatComRobot : Envoi message status...\n");
                if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
                    message->free(message);
                    IF_DEBUG rt_printf("t_check_etatComRobot : Echec envoi message status !\n");
                } else IF_DEBUG rt_printf("t_check_etatComRobot : Message status envoyé\n");

            } else {
                rt_mutex_release(&mutexCheck_etatComRobot);
                statusRobot = STATUS_OK;
            }
        }
    }

    //IF_DEBUG rt_printf("t_check_etatComRobot : compteur : %d\n", cpt);
    return statusRobot;
}