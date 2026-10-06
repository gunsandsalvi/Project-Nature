// The field of calibration scene C1 (A18.1, α2.2a): the ground family's shader with the full material, the
// one shared light function (A4.3) over its texture read, so C1 times what a pixel of the world will cost.
// α2.3b lights every family so, and the ground's shader becomes this. Implements PLT-04 and PRE-30.
shader_type spatial;

#define KD_LIGHT
#include "res://look/ground.gdshaderinc"
