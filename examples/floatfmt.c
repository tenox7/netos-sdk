#include <stdio.h>
int main(void)
{
	printf("pi      = %f\n", 355.0 / 113.0);
	printf("prec    = %.2f %.0f %.9f\n", 3.14159, 2.71828, 1.0 / 3.0);
	printf("width   = |%10.3f|%-10.3f|%+.2f|\n", 1.5, 1.5, 1.5);
	printf("neg     = %f\n", -0.125);
	printf("sci     = %e %E\n", 12345.678, 0.00042);
	printf("zero    = %f %.3f\n", 0.0, 0.0);
	printf("big     = %f\n", 1e20);
	return 0;
}
