// Calibration scene C5's field with fire shadows from a small map for each fire (A4.5, A18.1): the field
// of C1 with each fire's light read once from its fire's map. Implements PLT-04 and PRE-30.
shader_type spatial;

#define KD_LIGHT
#define KD_FIRE_MAP
#include "res://look/ground.gdshaderinc"
