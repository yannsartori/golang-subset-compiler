package main
/*
* Yann
* This program converts a (hardcoded) number in a particular base (hardcode) to another (hardcoded) base
* This utilizes short hand declarations, single expression for loops, 3 expression for loops, string conversion
* runes, strings, bytes, int, expression declaration, mod, string concatenation, built in methods, casts
*/

func main() {
	source := 13;
	dest := 23;
	num := "1075"
	println(baseOneToBaseTwo(num, source, dest))
}
func numToDecimal(num string, base int) int{
	result := 0
	for i := 0; i < len(num); i++ {
		result = result * base + charToInt(num[i]);
	}
	return result;
}
func decimalToBase(val int, base int) string {
	tempResult := "";
	for val != 0 {
		mod := val % base
		tempResult += intToChar(mod)
		val /= base
	}
	result := ""
	for i := 0 ; i < len(tempResult); i++ {
		result = string(tempResult[i]) + result;
	}
	return result;
}

func charToInt(c byte) int {
	if '0' <= c && c <= '9' {
		return (int) (c - '0')
	} else {
		return (int) (c - 'A' + 10)
	}
}
func intToChar(mod int) string {
	if 0 <= mod && mod <= 9 {
		return string('0' + mod);
	} else {
		return string('A' + (mod - 10));
	}
}
func baseOneToBaseTwo(num string, source int, dest int) string {
	decimal := numToDecimal(num, source);
	return decimalToBase(decimal, dest);
}

