package tests

import "fmt"

var blink = []func(id byte) error{
	func(id byte) error {
		fmt.Println("This is test 1")
		return nil
	},
	func(id byte) error {
		fmt.Println("This is test 2")
		return nil
	},
}
