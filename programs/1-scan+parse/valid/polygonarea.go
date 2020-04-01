

package polygonarea

type point struct {x float64; y float64;}

func signedTriangleArea(pointA, pointB, pointC point) float64 {
	var vec1 point
	vec1.x = pointA.x - pointB.x
	vec1.y = pointA.y - pointB.y
	var vec2 point
	vec2.x = pointC.x - pointB.x
	vec2.y = pointC.y - pointB.y
	return (vec1.x * vec2.y - vec1.y * vec2.x)/2.
}

func main() {
	var polygon [5]point
	polygon[0].x = 0.
	polygon[0].y = 0.
	polygon[1].x = 1.
	polygon[1].y = 0.
	polygon[2].x = 2.
	polygon[2].y = 1.
	polygon[3].x = 1.
	polygon[3].y = 2.
	polygon[4].x = 0.
	polygon[4].y = 1.
	
	if len(polygon) < 3 {
		println("That's not really a polygon")
	} else {
		var size float64
		var curIndex int = 1
		for curIndex < len(polygon) - 1 {
			size += signedTriangleArea(polygon[0], polygon[curIndex], polygon[curIndex + 1])
			curIndex ++
		}
		
		if size == 0. {
			println("that was weird...")
		} else if size > 0. {
			print("clockwise: ")
		} else {
			print("counter-clockwise: ")
			size = -size
		}
		
		println(size)
		
	}
	
}
