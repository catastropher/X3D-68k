#include <iostream>
#include <cmath>
#include <cstdio>

using namespace std;

int main() {
    printf("fp16x16 sintab[] = {\n");
    
    for(int i = 0; i < 32; ++i)
    {
        printf("    ");
        
        for(int j = 0; j < 8; ++j)
        {
            int angleInBase256 = i * 16 + j;
            double angleInDegrees = angleInBase256 * 360.0 / 256.0;
            double angleInRadians = angleInDegrees * 3.1415926535 / 180.0;
            int sineAsFp16x16 = sin(angleInRadians) * (1 << 16);
            
            printf("% -6d", sineAsFp16x16);
            
            if(angleInBase256 != 255)
                printf(", ");
        }
        
        printf("\n");
    }
    
    printf("};\n");
}

