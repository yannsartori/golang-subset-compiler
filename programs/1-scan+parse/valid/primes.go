// (Neil Pereira)Sieve of Erasthosthenes 

package main


func printPrimes (n int){
	if (n < 0){
    
		println("n must be >= 0")

		return
	}
	
	
	var nums []int
	//Add all numbers less than n to list
	for i:= 0; i < n ; i++ {
		nums = append(nums,i)
	}
	
	//Set all multiples of primes to 0
	for i := 2; i < n ; i++{
		k := 2
		for j := i; j*k < n ; j++ {
			nums[j*k] = 0
			k++
		}
	}
	
	//All non zero entries are printed(ie the prime numbers)
	for i := 2; i < n ; i++{
		if nums[i] != 0{
			println(i);
		}
	}	
}

func main() {
	
	
	printPrimes(500);

