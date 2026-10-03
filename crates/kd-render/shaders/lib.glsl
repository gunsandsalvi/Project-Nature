// Shared by every program (A11.13): it follows the generated defines, so it may use any of them.
// The categories, packing, the Bayer pattern, light steps and haze levels join here from α01a.

// The art pixel this fragment covers in the bound target, counted from its bottom-left corner.
ivec2 art_pixel() {
    return ivec2(gl_FragCoord.xy);
}
