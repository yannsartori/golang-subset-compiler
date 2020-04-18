//~11

package goosberry;

func A(_, a, _ int) {
	print(a);
}
func main() {
	var x struct{_, a, _ int;}
	x.a = 1;
	A(520,1,520);
	print(x.a);
}