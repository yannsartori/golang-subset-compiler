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
func main() {
	var poly []double; 
	poly = append(poly, 1.)
	poly = append(poly, 2.)
	poly = append(poly, 3.)
	poly = append(poly, 4.)
	precision := 50
	deriv := deriv(poly)
	var guess double = 2.;
	for i := 0 ; i < precision; i++ {
		guess -= polyEvaluate(poly, guess) / polyEvaluate(deriv, guess);
	}
	println(guess);
	
}
func deriv(poly []double) []double {
	var result []double;
	result = append(result, 0)
	for i := 1; i < len(poly); i++ {
		result = append(result, poly[i - 1] * (double) (len(poly) - i));
	}
	return result; 
}
func polyEvaluate(poly []double, x double) double {
	var result double = .0
	for i:= 0 ; i < len(poly); i++ {
		result += poly[i] * power(x, len(poly) - i - 1);
	}
	return result;
}
func power(base double, exp int) double {
	var result double = 1.0
	i := 0
	for i < exp {
		result *= base;
		i = i + 1
	}
	return result
}



