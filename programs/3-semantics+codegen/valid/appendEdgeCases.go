//~442240

package alfalfa;

func init() {
	var a, b []int
	a = append(a, 0)
	b = a
	a = append(a, 1)
	a[0] = 4
	print(a[0], b[0])
}
func init() {
	var a, b []int
	a = append(a, 0)
	b = a
	a = append(a, 1)
	b = append(b, 2)
	print(a[1], b[1])
}
func init() {
	var a,b []int 
	a = append(a, 0) 
	a = append(a, 1) 

	b = a
	
	a = append(a, 2) 
	a[0] = 4
	print(a[0], b[0])
}
func main() {}