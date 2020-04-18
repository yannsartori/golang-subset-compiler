//~010

package tamarind;

var a [5]int
var b []int
var c struct { f int; }

func A() [5]int { return a; }
func B() []int { return b; }
func C() struct {f int; } { return c; }

func main() {
	b = append(b, 0)
	var d,e,f = A(), B(), C()

	d[0], e[0], f.f = 1, 1, 1
	print(a[0], b[0], c.f)

}