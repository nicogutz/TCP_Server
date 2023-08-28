/**
 * \author Nicolas Gutierrez Suarez
 */

#ifndef DATAMGR_H_
#define DATAMGR_H_

#include <stdlib.h>
#include <stdio.h>

#include "config.h"

#ifndef RUN_AVG_LENGTH
#define RUN_AVG_LENGTH 5
#endif

#ifndef DSET_MAX_TEMP
#define DSET_MAX_TEMP 25
#endif

#ifndef DSET_MIN_TEMP
#define DSET_MIN_TEMP 10
#endif


typedef struct element element_t;

/**
 *  This method holds the core functionality of the datamgr. It reads sensor data from the shared buffer until
 *  the sensor id = 0, in which case it frees the memory and exits. It also calculates the running average of the
 *  sensors and logs if any is bigger than -DSET_MAX_TEMP or smaller than -DSET_MAX_TEMP.
 */
void *datamgr_init();

#endif  //DATAMGR_H_
