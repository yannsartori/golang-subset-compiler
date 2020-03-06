//Yann Sartori
//This checks to make sure that types match in expressions (e.g. addition)

package main
func main() {
	type intA int
	type intB int
	var x intA = intA(1) + intB(2)
}
