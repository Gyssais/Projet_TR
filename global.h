/* 
 * File:   global.h
 * Author: pehladik
 *
 * Created on 12 janvier 2012, 10:11
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

#include "includes.h"

/* @descripteurs des tâches */
extern RT_TASK tRecevoirMoniteur;
extern RT_TASK tConnect;
extern RT_TASK tMove;
extern RT_TASK tEnvoyer;
extern RT_TASK tWatchdog;
extern RT_TASK tTraitement_img;
extern RT_TASK tEtat_bat;

/* @descripteurs des mutex */
extern RT_MUTEX mutexEtatComMoniteur;
extern RT_MUTEX mutexEtatComRobot;
extern RT_MUTEX mutexMove;
extern RT_MUTEX mutexActionArena;
extern RT_MUTEX mutexPositionRobot;
extern RT_MUTEX mutexServeur;
extern RT_MUTEX mutexRobot;
extern RT_MUTEX mutexContinu;
extern RT_MUTEX mutexCheck_etatComRobot;


/* @descripteurs des sempahore */
extern RT_SEM semConnecterRobot;
extern RT_SEM semStartWatchdog;

/* @descripteurs des files de messages */
extern RT_QUEUE queueMsgGUI;

/* @variables partagées */
extern int etatComMoniteur;
extern int etatComRobot;
extern int continu;
extern DMovement *move;
extern DAction * actionArena;
extern DPosition *positionRobot;
extern DServer *serveur;
extern DRobot *robot;

/* @constantes */
extern int MSG_QUEUE_SIZE;
extern int PRIORITY_TRECEVOIR_MONITEUR;
extern int PRIORITY_TCONNECT;
extern int PRIORITY_TMOVE;
extern int PRIORITY_TENVOYER;
extern int PRIORITY_TWATCHDOG;
extern int PRIORITY_TTRAITEMENT_IMG;
extern int PRIORITY_TETAT_BAT;

#define NO_ACTION_ARENA 0

#endif	/* GLOBAL_H */

