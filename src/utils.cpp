#include "utils.h"
#include <math.h>

float roundTo(float number, float precision) {
    return (float) (floor(number * (1.0f/precision) + 0.5)/(1.0f/precision));
}
