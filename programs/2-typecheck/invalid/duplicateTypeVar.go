//Yann Sartori
//This checks to make sure a type cannot be declared in the same scope with the same name as a var

package main;
func main() {
	causeAnError := 1
	type causeAnError int
}
