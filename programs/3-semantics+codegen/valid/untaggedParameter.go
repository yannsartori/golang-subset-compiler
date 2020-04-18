//~520
package cantaloupe
func A(x struct{x int;}) {
	x.x = 520;
	print(x.x);
}
func main() {
	var x struct{x int;}
	A(x);
}