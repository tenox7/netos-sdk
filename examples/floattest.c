#include <stdio.h>

int main(void)
{
	double a = 355.0, b = 113.0, pi;
	float f = 1.5f;
	long long big = 1000000LL;

	pi = a / b;
	printf("355/113        = %d.%06d\n", (int) pi, (int) ((pi - (int) pi) * 1000000));
	printf("1.5f * 3       = %d\n", (int) (f * 3));
	printf("long long mul  = %d\n", (int) (big * 3 / 1000));
	printf("double compare = %d\n", a > b);
	return 0;
}
