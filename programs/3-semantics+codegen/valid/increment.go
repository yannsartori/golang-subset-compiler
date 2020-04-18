//~hi1

package pomegranate

func A() int{
	print("hi")
	return 0
}
func main() {
	var a [10]int
	a[A()]++
	print(a[0]);
}