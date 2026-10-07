// The stand-in area's strip the river runs in (A4.6, A4.5, A5.3): the ground's tiles and versions through the ladder, and
// the river's bed below the water's level, tinted by depth, with its shore in whole texture pixels. The body is
// land.gdshaderinc with KD_RIVER defined. Implements PRE-22, PRE-23 and PRE-26.
shader_type spatial;

#define KD_RIVER
#include "res://look/land.gdshaderinc"
