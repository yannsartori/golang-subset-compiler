//Yann Sartori
//This checks struct assignment matches the field type
package main
func main() {
	type myStruct struct {
		x int
	}
	var y myStruct;
	y.x = 'a'
}
