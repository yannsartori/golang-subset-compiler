
package minesweeper		//Generates a 7x7 minesweeper board with the mines and numbers pseudorandomly


var seedA, seedB int = 3745, 7708




func randInt(lower, upper int) int {	// Generates pseudo-random numbers based on the two seeds
	seedA = seedA * seedA * seedA	// It seems like it works?
	seedA = seedA % seedB
	if seedA < 100 {
		seedA = -seedA + 2026
	}
	seedB = seedB - 7
	if seedB < 3761 {
		seedB = seedB + 6427
	}
	var x = lower + (seedA % (upper-lower))
	return x
}

func addMine(board [7][7]int) [7][7]int {
	var x,y = randInt(0, 7), randInt(0, 7)
	for board[x][y] == -1 {	// don't put two mines on top of each other
		x,y = randInt(0, 7), randInt(0, 7)
	}
	board[x][y] = -1		//create a new mine
	for i := -1; i<= 1; i++ {
		for j := -1; j <=1; j++ {
			if x+i >= 0 && x+i < 7 {	//Check to make sure we're on the board
				if y+j >=0 && y+j < 7 {
					if board[x+i][y+j] != -1 { //mines don't talk about other mines
						board[x+i][y+j] = board[x+i][y+j] +1 //blank squares tell you how many mines are adjacent
					}
				}
			}
		}
	}
	return board
}


func generateBoard(numMines int) [7][7]int {
	var board [7][7]int
	for i := 0; i < numMines; i++ {
		board = addMine(board)
	}
	return board
}

func printBoard(board [7][7]int) {
	for i := 0; i < 7; i++ {
		for j := 0; j < 7; j++ {
			if board[i][j] == -1 {
				print("*")
			} else {
				print(board[i][j])	// this should only ever be one digit
			}
		}
		println();
	}
}





func main() {
	var numMines int = 7
	var board = generateBoard(numMines)
	printBoard(board)
}
