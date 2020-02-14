package main;
/*
* Yann
* Checks to make sure function declarations cannot occur
* within some scope
* Checks the parser
*/
func main() {
	func failure() {
		return 0;
	}
}
