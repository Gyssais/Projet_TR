/*
 * File:   global.h
 * Author: pehladik
 *
 * Created on 21 avril 2011, 12:14
 */

#include "global.h"

RT_TASK tRecevoirMoniteur;
RT_TASK tConnect;
RT_TASK tMove;
RT_TASK tEnvoyer;
RT_TASK tWatchdog;
RT_TASK tTraitement_img;
RT_TASK tEtat_bat;

RT_MUTEX mutexEtatComRobot;
RT_MUTEX mutexEtatComMoniteur;
RT_MUTEX mutexMove;
RT_MUTEX mutexPositionRobot;
RT_MUTEX mutexContinu;
RT_MUTEX mutexActionArena;
RT_MUTEX mutexCheck_etatComRobot;
RT_MUTEX mutexServeur;
RT_MUTEX mutexRobot;

RT_SEM semConnecterRobot;
RT_SEM semStartWatchdog;

RT_QUEUE queueMsgGUI;

int etatComMoniteur;
int etatComRobot;
int continu;
DRobot *robot;
DMovement *move;
DServer *serveur;
DPosition *positionRobot;
DAction * actionArena;


int MSG_QUEUE_SIZE = 10;

int PRIORITY_TRECEVOIR_MONITEUR = 70;
int PRIORITY_TCONNECT = 20;
int PRIORITY_TMOVE = 15;
int PRIORITY_TENVOYER = 25;
int PRIORITY_TWATCHDOG = 90;
int PRIORITY_TTRAITEMENT_IMG = 42;
int PRIORITY_TETAT_BAT = 10;
