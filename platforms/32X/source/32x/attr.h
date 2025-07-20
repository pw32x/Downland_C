#ifndef __ATTR_H__
#define __ATTR_H__

#define ATTR_DATA_OPTIMIZE_NONE __attribute__((section(".data"), aligned(16), optimize("O1")))
#define ATTR_DATA_CACHE_ALIGN __attribute__((section(".data"), aligned(16), optimize("Os")))
#define ATTR_OPTIMIZE_SIZE __attribute__((optimize("Os")))
#define ATTR_OPTIMIZE_EXTREME __attribute__((optimize("O3", "no-align-loops", "no-align-functions", "no-align-jumps", "no-align-labels")))

#endif
