//~010

package xigua;

func A(a [5]int, b []int, c struct{f int;}) {
	a[0] = 1
	b[0] = 1
	c.f = 1
}
func main() {
	var a [5]int
	var b []int
	b = append(b, 0)
	var c struct { f int; }
	A(a,b,c);
	print(a[0],b[0],c.f)
}