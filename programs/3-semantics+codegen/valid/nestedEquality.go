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
	/* This was to check if the typechains, arraycopying, struct copying, were all working */
	/* To spare you the trouble, they do work. We generate duplicate casts sometimes but that is fine.*/
	q := a;
	w := a[0];
	e := a[0][1]
	r := a[0][1].b 
	s := a[0][1].b[1]
	t := a[0][1].b[1][2]
	u := a[0][1].b[1][2].a
	a[0][1].b[1][2].a = 1;
	print(a == b);
	b[0][1].b[1][2].a = 1;
	print(a == b);

}