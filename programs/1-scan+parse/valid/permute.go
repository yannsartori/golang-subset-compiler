//Neil Permutes hardcoded list of integers


package main



func concat(list [][][]int) [][]int{
	var temp [][]int;
	for i := 0 ; i < len(list) ; i++{
		for j := 0; j < len(list[i]); j++ {
			temp = append(temp,list[i][j])
		}
	}
	return temp
}

func insert (n int, position int,numList []int) []int{
	var toReturn []int;
	for i := 0; i < len(numList)  ; i++ {
		if (i == position){
			toReturn = append(toReturn,n)
		}
		
		
		toReturn = append(toReturn,numList[i])
	}
	
	if (len(toReturn) != len(numList) + 1){
		toReturn = append(toReturn,n)
	}
	
	
	
	return toReturn;
}


func interleave (n int,numList []int) [][]int{
	var toReturn [][]int;
	switch k := len(numList); k {
		case 0 : var temp []int;
			temp = append(temp,n)
			toReturn = append(toReturn,temp)
			return toReturn
		default : 
			for i := 0; i < k+1 ; i++{
				toReturn = append(toReturn,insert(n,i,numList))
		
			}
			
			return toReturn
		
	}
}



func permute(numList []int) []([]int) {
	var x [][]int
	
	switch n:= len(numList);n{
		case 0 : var y []int
			x = append(x,y)
			return x
		default : var temp []int;
				for i := 1; i < n; i++{
					temp = append(temp,numList[i])
				}
				var temp1 [][]int = permute(temp);
				var temp2 [][][]int;
				for i := 0; i < len(temp1); i++{
					temp2 = append(temp2,interleave(numList[0],temp1[i]))
					
				}
				
				return concat(temp2)
		

	}
	
}

func main() {

	var numList []int;
	numList = append(numList,1);
	numList = append(numList,2);
	numList = append(numList,3);


	var result [][]int
	result = permute(numList)
	for i := 0; i < len(result); i++{
		for j := 0; j < len(result[i]); j++{
		
			print(result[i][j])
		}
		println();
		
	}
	
}
