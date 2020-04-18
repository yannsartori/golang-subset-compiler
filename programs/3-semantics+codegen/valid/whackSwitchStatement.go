//~
package starFruit;

func A() int {
	return 1;
}

func main() {
	a := 0
	switch A() {
	case a, A():
	case 2:	
		print(1)
		break
	default:
		print(2)
	}
}