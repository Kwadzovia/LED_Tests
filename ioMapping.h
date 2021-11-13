/*
 * ioMapping.h
 *
 *  Created on: Oct 25, 2021
 *      Author: selik
 */

#ifndef IOMAPPING_H_
#define IOMAPPING_H_

#if COMPILING_FOR_V1
#include "ioMapping_v1.h"
#elif COMPILING_FOR_V2
#include "ioMapping_v2.h"
#else
#error "NO IO MAP DEFINED IN PREPROCESSOR"
#endif /* COMPILING_FOR_V1 */

#endif /* IOMAPPING_H_ */
