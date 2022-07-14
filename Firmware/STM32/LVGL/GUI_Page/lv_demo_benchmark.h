/**
 * @file lv_demo_benchmark.h
 *
 */

#ifndef LV_DEMO_BENCHMARK_H
#define LV_DEMO_BENCHMARK_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lvgl.h"
//#include "lv_ex_conf.h"
#else
#include "../../../lvgl/lvgl.h"
#endif
/*********************
 *      DEFINES
 *********************/
#define LV_USE_DEMO_BENCHMARK 1
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_demo_benchmark(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DEMO_BENCHMARK_H*/
