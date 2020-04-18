//~1248

package rambutan;

func main() {
	for a, b := 0, 1; a < 5; a, b = a + 1, b * 2{
		if  b > 8  {
			continue;
		}
		a := 5
		print(b)
	}
}