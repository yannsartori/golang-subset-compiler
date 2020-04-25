//~Cap: 0 , len: 0
//~Cap: 2 , len: 1
//~Cap: 2 , len: 2
//~Cap: 4 , len: 3
//~Cap: 4 , len: 4
//~Cap: 8 , len: 5
//~Cap: 8 , len: 6
//~Cap: 8 , len: 7
//~Cap: 8 , len: 8
//~Cap: 16 , len: 9

package zucchini

func main() {
	 var a []int

	 for i := 0; i < 10; i++ {
		 println("Cap:", cap(a), ", len:", len(a))
		a = append(a, 0)
	}
}