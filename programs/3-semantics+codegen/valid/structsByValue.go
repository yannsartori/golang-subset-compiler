//~520
package dewberry;
type B struct{x int;}
func A(x B) {
	x.x = 5000000
}
func main() {
	var x B
	x.x = 520;
	A(x);
	print(x.x);
}