#include <stdio.h>

int main(){
	int a = 1;
	int b = 0;
	int c = 1;
	int result = ((a & b) ^ (b & c)) ^ c;
	printf("result: %d\n", result);
}
