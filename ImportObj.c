#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ImportObj.h"
#include "calcs.h"  // Include your calcs header

#define MAX_VERTS 50000
#define MAX_TRIS  100000

Triangle* LoadObjTriangles(const char* filename, int* out_count) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open OBJ file: %s\n", filename);
        *out_count = 0;
        return NULL;
    }

    Vec3* verts = malloc(sizeof(Vec3) * MAX_VERTS);
    int vert_count = 0;

    Triangle* tris = malloc(sizeof(Triangle) * MAX_TRIS);
    int tri_count = 0;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            Vec3 v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            // Invert Y axis here
            v.y = -v.y;

            if (vert_count < MAX_VERTS) {
                verts[vert_count++] = v;
            }
        } else if (line[0] == 'f' && line[1] == ' ') {
            int i0 = 0, i1 = 0, i2 = 0;

            char* token = strtok(line + 2, " \n");
            if (token) i0 = atoi(token);
            token = strtok(NULL, " \n");
            if (token) i1 = atoi(token);
            token = strtok(NULL, " \n");
            if (token) i2 = atoi(token);

            if (i0 > 0 && i1 > 0 && i2 > 0 && tri_count < MAX_TRIS) {
                // Swap i1 and i2 to invert winding (flip normals)
                Triangle tri = {
                    .v0.pos = verts[i0 - 1],
                    .v1.pos = verts[i2 - 1],  // swapped
                    .v2.pos = verts[i1 - 1],  // swapped
                };
                tris[tri_count++] = tri;
            }
        }
    }

    fclose(file);
    free(verts);

    printf("Loaded %d triangles from %s\n", tri_count, filename);

    *out_count = tri_count;
    return tris;
}