//~Cap: 0, len: 0\nCap: 2, len: 1\nCap: 2, len: 2\nCap: 4, len: 3\nCap: 4, len: 4\nCap: 8, len: 5\nCap: 8, len: 6\nCap: 8, len: 7\nCap: 8, len: 8\nCap: 16, len: 9\n

package zucchini

func main() {
	 var a []int

	 for i := 0; i < 10; i++ {
		 println("Cap:", cap(a), ", len:", len(a))
		a = append(a, 0)
	}
}