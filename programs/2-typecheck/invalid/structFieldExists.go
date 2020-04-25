//Yann Sartori
//This checks to make sure we access struct fields that exist

package main
func main() {
	type myStruct struct {
		x int
	}
	var y myStruct;
	y.notHere = 5
}
