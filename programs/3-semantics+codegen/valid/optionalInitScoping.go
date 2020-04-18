//~3false

package quince;

func main() {
	a := false
	if a := false; a {
		print(1)
	} else if a := true; !a {
		print(2)
	} else if a {
		print(3)
	} else {
		print(4)
	}
	print(a)
}