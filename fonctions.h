/* 
 * File:   fonctions.h
 * Author: pehladik
 *
 * Created on 12 janvier 2012, 10:19
 */

#ifndef FONCTIONS_H
#define	FONCTIONS_H

#include "global.h"
#include "includes.h"

#ifdef	__cplusplus
extern "C" {
#endif
        void connecter (void * arg);
        void recevoir_moniteur(void *arg); // Communication avec le robot : 
        void deplacer(void *arg);
        void envoyer_moniteur(void *arg); // Envoi de messages au moniteur
        void watchdog(void *arg); // Relance du watchdog du robot
        void traitement_image(void *arg); 
        void etat_batterie(void *arg);
#ifdef	__cplusplus
}
#endif

#endif	/* FONCTIONS_H */

