#ifndef IMPORT_OBJ_H
#define IMPORT_OBJ_H

#include "calcs.h"

// Load triangles from an .obj file.
// Returns malloc'd array of Triangle. Writes count to out_count.
// Caller must free the returned array.
Triangle* LoadObjTriangles(const char* filename, int* out_count);

#endif
