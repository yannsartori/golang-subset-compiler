//~ right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->down->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->right->up->up->up->up->right->down->down->down->down->down->


package main

type Point struct{
	wasVisited bool;
	x,y int;
	label string;
}

type Maze struct{
	mazeMap [100][100]Point
}

type Move struct{
	direction string
}

var maze Maze

func withinBounds(x,y int) bool{
	return (0 <= x && x < 100 && 0 <= y && y < 100)
}

var up Move
var down Move
var left Move
var right Move

func init(){
	for i := 0; i < 100; i++ {
		for j := 0; j < 100; j++ {		
			maze.mazeMap[i][j].wasVisited = false
			maze.mazeMap[i][j].x = j 
			maze.mazeMap[i][j].y = i 
			maze.mazeMap[i][j].label = "X" 
		}
	}

	maze.mazeMap[99][99].label = "END"

	up.direction = "up"
	down.direction = "down"
	left.direction = "left"
	right.direction = "right"
}

func findPath(x,y int, move Move, path []Move) [][]Move{
	var paths [][]Move
	if (!withinBounds(x,y)){
		return paths;
	}

	if (maze.mazeMap[y][x].label =="END"){
		return append(paths,append(path,move));
	}else if (maze.mazeMap[y][x].wasVisited){
		return paths
	}
	maze.mazeMap[y][x].wasVisited = true

	var upPath,leftPath,rightPath,downPath [][]Move

	extendedPath := append(path,move)

	switch move.direction{
		case "up":
			upPath = findPath(x-1,y,up,extendedPath)
			leftPath = findPath(x,y-1,left,extendedPath)
			rightPath = findPath(x,y+1,right,extendedPath)
		case "down":
			downPath = findPath(x+1,y,down,extendedPath)
			leftPath = findPath(x,y-1,left,extendedPath)
			rightPath = findPath(x,y+1,right,extendedPath)
		case "left":
			upPath = findPath(x-1,y,up,extendedPath)
			downPath = findPath(x+1,y,down,extendedPath)
			leftPath = findPath(x,y-1,left,extendedPath)
		case "right":
			upPath = findPath(x-1,y,up,extendedPath)
			downPath = findPath(x+1,y,down,extendedPath)
			rightPath = findPath(x,y+1,right,extendedPath)
	}



	for i := 0; i < len(upPath) ;i++{
		paths = append(paths,upPath[i])
	}
	for i := 0; i < len(downPath) ;i++{
		paths = append(paths,downPath[i])
	}
	for i := 0; i < len(leftPath) ;i++{
		paths = append(paths,leftPath[i])
	}
	for i := 0; i < len(rightPath) ;i++{
		paths = append(paths,rightPath[i])
	}
	

	return paths
}

var result [][]Move

func init(){
	var initPath []Move
	rightPath := findPath(0,0,right,initPath);
	downPath := findPath(0,0,down,initPath);

	for i := 0; i < len(downPath) ;i++{
		result = append(result,downPath[i])
	}

	for i := 0; i < len(rightPath) ;i++{
		result = append(result,rightPath[i])
	}

}



func main(){
	for i :=0; i < len(result) ;i++{
		for j := 0; j < len(result[i]); j++{
			if j % 20 == 0{
				print(result[i][j].direction,"->")
				}
			
	}
}
	println();
}
