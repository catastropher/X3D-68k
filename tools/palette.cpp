#include <cstdio>


using namespace std;

int main() {
    printf("uint8 quake_color_palette[256][3] = {");
    
    for(int i = 0; i < 256; ++i) {
        int r, g, b;
        scanf("R: %d, G: %d, B: %d\n", &r, &g, &b);
        printf("\n    { %3d, %3d, %3d }%s", r, g, b, (i != 255 ? "," : ""));
    }
    
    printf("\n};\n");
}
