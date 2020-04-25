//~-6.531549e+01
package main;
/*
* Yann
* This program finds a zero of a ((hardcoded)) polynomial using Netwon's method
* Utilizes type casting, slices (including the built-ins), ommission and inclusion
* of semi colons, short declaration, normal declaration, functions, expression statements
* various expressions, optional 0 before or after decimal in floats, for loops in the 3
* statement variant as well as in the single (while) varient, indexing, type casts
*/
type double float64
func power(base double, exp int) double {
	var result double = double(1.0)
	i := 0
	for i < exp {
		result *= base;
		i = i + 1
	}
	return result
}
func deriv(poly []double) []double {
	var result []double;
	result = append(result, double(0))
	for i := 1; i < len(poly); i++ {
		result = append(result, poly[i - 1] * (double) (len(poly) - i));
	}
	return result; 
}
func polyEvaluate(poly []double, x double) double {
	var result double = double(.0)
	for i:= 0 ; i < len(poly); i++ {
		result += poly[i] * power(x, len(poly) - i - 1);
	}
	return result;
}
func main() {
	var poly []double; 
	poly = append(poly, double(1.6543263))
	poly = append(poly, double(2.26432542))
	poly = append(poly, double(3.4312434))
	poly = append(poly, double(451531.))
	precision := double(20000000)
	deriv := deriv(poly)
	var guess double = double(2.);
	for i := 0 ; double(i) < precision; i++ {
		guess -= polyEvaluate(poly, guess) / polyEvaluate(deriv, guess);
	}
	println(guess);
	
}