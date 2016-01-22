/* $Id: barr_example.mc,v 1.1 2006/05/15 21:57:25 hammes Exp $
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

void subr (int64_t Buff[], int64_t Res[], int sz, int iter, Barrier_group_t barrier, int mapnum) {
    OBM_BANK_A (AL, int64_t, MAX_OBM_SIZE)
    OBM_BANK_B (BL, int64_t, MAX_OBM_SIZE)
    Stream_64 S0;
    int i;

    for (i=0; i<iter; i++) {
	// wait for CPU to fill buffer
        Barrier_Wait (barrier, 0);

	#pragma src parallel sections
	{
	    #pragma src section
	    {
	    streamed_dma_cpu_64 (&S0, PORT_TO_STREAM, Buff, sz*8);
	    }

	    #pragma src section
	    {
	    int k;
	    int64_t v, ac;

	    ac = 0;
	    for (k=0; k<sz; k++) {
		  get_stream_64 (&S0, &v);
		  ac ^= v;
		}

	    AL[i] = ac;
	    }
	}

	// let CPU know it can start rewriting the buffer
        Barrier_Wait (barrier, 0);
    }


    buffered_dma_cpu (OBM2CM, PATH_0, AL, MAP_OBM_stripe(1,"A"), Res, 1, iter*sizeof(int64_t));
    }

