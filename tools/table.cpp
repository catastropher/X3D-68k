#include <iostream>
#include <cmath>
#include <cstdio>

#define PI 3.1415926535

#define DEG_TO_RAD(_angle) ((_angle) * (PI / 180.0))
#define D256_TO_D360(_angle) ((_angle) * (360.0 / 256.0))

double get_sin(int angle) {
	return sin(DEG_TO_RAD(D256_TO_D360(angle)));
}


using namespace std;

int main() {
	int angle = 0;
	int row = 10;
	
	cout << "const short sintab[256] = {";
	
	for(int i = 0; i < 256; i++) {
		if((i % row) == 0) {
			printf("\n\t");
		}
		
		int num = get_sin(i) * 32768;
		
		if(num > 32767)
			num = 32767;
		else if(num < -32768)
			num = -32768;
		
		printf("%6d", num);
		
		if(i != 256 - 1)
			printf(", ");
	}
	
	cout << "\n};";
}
	