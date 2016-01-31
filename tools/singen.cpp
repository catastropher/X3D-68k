#include <iostream>
#include <cmath>
#include <cstdio>

using namespace std;

int main() {
	printf("int16 sintab[] = {\n");
	
	for(int i = 0; i < 256; ++i) {
		int val = sin(i * (360.0 / 256.0) * (3.1415926535 / 180.0)) * 32768.0;

		if(val == 32768)
			val = 32767;
		else if(val == -32769)
			val = -32768;

		printf("\t%d\n", val);
	}
	printf("}\n");
}
