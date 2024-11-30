package tests

var Tests = map[string][]func(id byte) error{
	"blink": blink,
}
