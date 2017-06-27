#include <lpc17xx.h>
#include "glcd.h"
#include <stdbool.h>




double newtown_raphson(double input){
		double a = input;
		double xn = 1/2*(input +1);
		while(abs(xn - input) > 1e-6){
				xn = 1/2*(xn+ a/xn);
		}
		return xn;
}
