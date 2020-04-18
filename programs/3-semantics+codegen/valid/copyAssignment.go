//~010
package nectarine;

func main() {
	var a, b [5]int
	b = a
	a[0] = 1
	var c,d []int
	c = append(c, 0)
	d = c
	c[0] = 1
	var e, f struct { f int; }
	f = e
	e.f = 1

	print(b[0], d[0], f.f);
}