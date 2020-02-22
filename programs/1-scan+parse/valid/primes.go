// (Neil Pereira)Sieve of Erasthosthenes 

package main
func printPrimes (){
	var nums [1000]int
	for i:= 0; i < 1000 ; i++ {
		nums[i] = i
	}
	
	for i := 2; i < 1000 ; i++{
		k := 2
		for j := i; j*k < 1000 ; j++ {
			nums[j*k] = 0
			k++
		}
	}
	
	for i := 2; i < 1000 ; i++{
		if nums[i] != 0{
			println(i);
		}
	}	
}

func main() {
	
	println("All primes less than 1000 are as follows")
	printPrimes();
}
