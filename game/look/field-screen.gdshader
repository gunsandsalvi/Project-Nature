// Calibration scene C5's field with fire shadows from a walk at half resolution (A4.5, A18.1): the field
// of C1 with the first three fires' light read by screen position from the half-resolution picture
// fire-screen.gdshader draws. Implements PLT-04 and PRE-30.
shader_type spatial;

#define KD_LIGHT
#define KD_FIRE_SCREEN
#include "res://look/ground.gdshaderinc"
