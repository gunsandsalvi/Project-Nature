// The solid cores of leaf cards (A4.6), cut nowhere, so the chip hides what is behind them as it does
// behind any solid thing: the cores of calibration scene C2's third way. Its body is
// leaves.gdshaderinc. Implements PRE-46.
shader_type spatial;
render_mode cull_disabled, specular_disabled;

#define KD_SOLID
#include "res://look/leaves.gdshaderinc"
