package main

import (
	"fmt"
	"os"

	"github.com/rafaelmartins/usbd-fs-stm32-tests/runner/internal/stlink"
)

func run(dev *stlink.Stlink) error {
	return nil
}

func main() {
	devices, err := stlink.Enumerate()
	if err != nil {
		fmt.Fprintf(os.Stderr, "error: %s", err)
	}

	failures := 0
	for _, dev := range devices {
		fmt.Fprintf(os.Stderr, "========================================\n")
		fmt.Fprintf(os.Stderr, "Running tests for %s\n", dev.Name)
		fmt.Fprintf(os.Stderr, "----------------------------------------\n")
		err := run(dev)
		fmt.Fprintf(os.Stderr, "----------------------------------------\n")
		if err == nil {
			fmt.Fprintf(os.Stderr, "PASSED (%s)\n", dev.Name)
			continue
		}
		failures++
		fmt.Fprintf(os.Stderr, "FAILED (%s): %s\n", dev.Name, err)
	}
	fmt.Fprintf(os.Stderr, "========================================\n")
	if failures == 0 {
		fmt.Fprintf(os.Stderr, "PASSED (%d/%d)\n", len(devices), len(devices))
		return
	}
	fmt.Fprintf(os.Stderr, "FAILED (%d/%d)\n", failures, len(devices))
	os.Exit(1)
}
