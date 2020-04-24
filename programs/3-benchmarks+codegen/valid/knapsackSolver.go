//~Take the following items: 33, 30, 29, 28, 25, 23, 21, 17, 15, 13, 12, 11, 7, 5, 3, 2. They weigh +2.897900e+01 and their total value is +9.239300e+01




package ballon_baby


var seedA, seedB int = 3745, 7708
func randInt() int {	// Generates pseudo-random numbers based on the two seeds
	if seedA < 100 {seedA = -seedA + 2026;}
	seedA = seedA * seedA * seedA	// It seems like it works?
	seedA = seedA % seedB
	if seedA < 0 {seedA = -seedA;}
	seedB = seedB - 7
	if seedB < 8402 {seedB = seedB + 6427;}
	var x = (seedA % 10000)
	return x
}




type Item struct{
	weight float64;
	value float64;
}

var pile [34] Item


func findBestValue(maxWeight float64, position int) []int {
	
	
	var picks []int
	
	if position >= 34 {
		return picks
	}
	
	if pile[position].weight > maxWeight {
		return findBestValue(maxWeight, position + 1)
	}
	
	include, exclude := findBestValue(maxWeight - pile[position].weight, position + 1), findBestValue(maxWeight, position + 1)
	
	var includeVal, excludeVal float64
	
	for i:=0; i<len(include); i++ {
		includeVal += pile[include[i]].value
	}
	includeVal += pile[position].value
	
	for i:=0; i<len(exclude); i++ {
		excludeVal += pile[exclude[i]].value
	}
	
	if includeVal > excludeVal {
		return append(include, position)
	} else {
		return exclude
	}
	
	
	
}


func main() {
	
	
	var chosen = findBestValue(30.0, 0)
	
	print("Take the following items: ")
	
	print(chosen[0])
	
	for i:=1; i<len(chosen); i++ {
		print(", ")
		print(chosen[i])
	} 
	print(". ");
	
	print("They weigh ")
	
	var x float64
	
	for i:=0; i<len(chosen); i++ {
		x += pile[chosen[i]].weight
	} 
	
	print(x)
	
	var y float64
	
	for i:=0; i<len(chosen); i++ {
		y += pile[chosen[i]].value
	} 
	
	print(" and their total value is ")
	println(y)

}




func init() {
	for i := 0; i<34; i++ {
		pile[i].value = float64(randInt())/1000.0
		pile[i].weight = float64(randInt())/1000.0
		/*
		print(i)
		print(": val ")
		print(pile[i].value)
		print("; weight ")
		println(pile[i].weight)
		*/
	}
}




