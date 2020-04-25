//Neil - printing a type that does not reolve to a printable base type
package main

type Point struct{
	x int
	y int
}

func main(){
	var p Point
	print(p)
}


