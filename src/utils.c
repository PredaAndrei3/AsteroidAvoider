#include "utils.h"

#include <stdlib.h>

int random_range(int a, int b) {
    return rand() % (b - a + 1) + a;
}