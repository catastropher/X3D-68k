#include <iostream>
#include <cmath>
#include <cstdio>

#define PI 3.1415926535

#define DEG_TO_RAD(_angle) ((_angle) * (PI / 360.0))
#define D256_TO_D360(_angle) ((_angle) * (360.0 / 256.0))

double get_sin(int angle) {
	return sin(DEG_TO_RAD(D256_TO_D360(angle)));
}


using namespace std;

int main() {
	int angle = 0;
	int row = 16;
	
	cout << "const short sintab[] = {";
	
	for(int i = 0; i < 256; i++) {
		if((i % row) == 0) {
			printf("\n\t");
		}
		
		printf("%6d", get_sin(i));
		
		if(i != 256 - 1)
			printf(", ");
	}
	
	cout << "\n};";
}
	