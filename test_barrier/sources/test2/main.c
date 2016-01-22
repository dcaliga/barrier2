/* $Id: main.c,v 1.1 2006/05/15 21:57:25 hammes Exp $
 *
 * Copyright 2003 SRC Computers, Inc.  All Rights Reserved.
 *
 *      Manufactured in the United States of America.
 *
 * SRC Computers, Inc.
 * 4240 N Nevada Avenue
 * Colorado Springs, CO 80907
 * (v) (719) 262-0213
 * (f) (719) 262-0223
 *
 * No permission has been granted to distribute this software
 * without the express permission of SRC Computers, Inc.
 *
 * This program is distributed WITHOUT ANY WARRANTY OF ANY KIND.
 */

#include <libmap.h>
#include <pthread.h>


void subr (int64_t Buff[], int64_t Res[], int sz, int iter, Barrier_group_t barrier, int mapnum);


void *proc0 (void *p);
void *proc1 (void *p);


int64_t *Buff;
int64_t *Res;
int64_t *ResCorrect;
Barrier_group_t barrier;
int SIZE;
int ITER;


int main (int argc, char *argv[]) {
    FILE *res_map, *res_cpu;
    int i;
    pthread_t thread0, thread1;

    if ((res_map = fopen ("res_map", "w")) == NULL) {
        fprintf (stderr, "failed to open file 'res_map'\n");
        exit (1);
        }

    if ((res_cpu = fopen ("res_cpu", "w")) == NULL) {
        fprintf (stderr, "failed to open file 'res_cpu'\n");
        exit (1);
        }

    if (argc < 3) {
        fprintf (stderr, "need size and iteration count as args\n");
        exit (1);
        }

    if (sscanf (argv[1], "%d", &SIZE) < 1) {
        fprintf (stderr, "need size and iteration count as args\n");
        exit (1);
        }

    if (sscanf (argv[2], "%d", &ITER) < 1) {
        fprintf (stderr, "need size and iteration count as args\n");
        exit (1);
        }

    if (map_allocate (1)) {
       fprintf (stdout, "Map allocation failed.\n");
       exit (1);
       }

    Buff = (int64_t*) Cache_Aligned_Allocate (2 * SIZE * sizeof (int64_t));
    Res = (int64_t*) Cache_Aligned_Allocate (ITER * sizeof (int64_t));
    ResCorrect = (int64_t*) Cache_Aligned_Allocate (ITER * sizeof (int64_t));

    barrier = Barrier_Allocate ();
    Barrier_Initialize (barrier, 0, 2);

    pthread_create (&thread0, NULL, &proc0, NULL);
    pthread_create (&thread1, NULL, &proc1, NULL);
    pthread_join (thread0, NULL);
    pthread_join (thread1, NULL);

    if (map_free (1)) {
        printf ("Map deallocation failed. \n");
        exit (1);
        }

    for (i=0; i<ITER; i++) {
        fprintf (res_cpu, "%lld\n", ResCorrect[i]);
        fprintf (res_map, "%lld\n", Res[i]);
        }

    exit (0);
    }


void *proc0 (void *p) {
    subr (Buff, Res, SIZE, ITER, barrier, 0);
    return NULL;
    }


void *proc1 (void *p) {
    int i, k, idx;
    int64_t ac;

    for (i=0; i<ITER; i++) {
	idx = (i & 1) ? SIZE : 0;

	// fill the buffer
	ac = 0;
	for (k=0; k<SIZE; k++) {
	    Buff[idx+k] = (((int64_t)random ()) << 32) | random ();;
	    ac ^= Buff[idx+k];
	    }

	ResCorrect[i] = ac;

	// let map know it can stream the buffer that was just written
	Barrier_Wait (barrier, 0);
        }

    return NULL;
    }

