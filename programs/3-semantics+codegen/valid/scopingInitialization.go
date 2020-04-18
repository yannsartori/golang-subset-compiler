//~
package farkleberry;
func main() {
	var x int;
	{
		var y = x;
		var x = x;
	}
}