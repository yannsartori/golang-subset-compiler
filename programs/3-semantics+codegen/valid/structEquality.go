//~truefalse

package kumquat;
func main() {
	var a, b struct {
		_ float64
		f int
	}
	print(a == b)
	a.f = 5;
	print(a == b);
}