//~truefalsetrue

package lychee;
//I wish I had some lychees rn :(

func main() {
	type A struct {
		a int;
		b float64;
	}
	type B struct {
		a int;
		_ float64;
		b [3][4]A
	}
	var a, b [1][2]B;
	print(a == b);
	a[0][1].b[1][2].a = 1;
	print(a == b);
	b[0][1].b[1][2].a = 1;
	print(a == b);

}