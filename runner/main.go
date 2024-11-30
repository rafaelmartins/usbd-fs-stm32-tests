package main

import (
	"errors"
	"fmt"
	"io/fs"
	"os"
	"path/filepath"

	"github.com/rafaelmartins/usbd-fs-stm32-tests/runner/internal/firmware"
	"github.com/rafaelmartins/usbd-fs-stm32-tests/runner/internal/serial"
	"github.com/rafaelmartins/usbd-fs-stm32-tests/runner/internal/stlink"
	"github.com/rafaelmartins/usbd-fs-stm32-tests/runner/tests"
)

func runDevice(dev *stlink.Stlink, testdir string) error {
	if err := dev.Open(); err != nil {
		return err
	}
	defer dev.Close()

	family, err := dev.Family()
	if err != nil {
		return err
	}

	s, err := serial.New(dev.Serial)
	if err != nil {
		return err
	}
	defer s.Close()

	for testName, cbs := range tests.Tests {
		firmwareName := "test-" + testName + "-" + family + ".hex"
		firmware := filepath.Join(testdir, firmwareName)
		if _, err := os.Stat(firmware); err != nil {
			if errors.Is(err, fs.ErrNotExist) {
				return fmt.Errorf("runner: firmware not found: %w: %s", err, firmwareName)
			}
			return err
		}

		fmt.Fprintf(os.Stderr, "==> Flashing: %s\n", firmwareName)
		if err := dev.Flash(firmware); err != nil {
			return err
		}

		if err := s.Sync(); err != nil {
			return err
		}

		cnt, err := s.GetTestCount()
		if err != nil {
			return err
		}

		if int(cnt) != len(cbs) {
			return fmt.Errorf("runner: number of tests implemented in firmware differs from tests implemented in runner: %d != %d", len(cbs), cnt)
		}

		for i, cb := range cbs {
			fmt.Fprintf(os.Stderr, "==> Test: %d\n", i+1)
			if err := s.ConfigureTest(byte(i + 1)); err != nil {
				return err
			}
			if err := cb(byte(i + 1)); err != nil {
				return err
			}
		}
	}
	return nil
}

func main() {
	if err := stlink.Init(); err != nil {
		fmt.Fprintf(os.Stderr, "error: %s", err)
		os.Exit(1)
	}

	devices, err := stlink.Enumerate()
	if err != nil {
		fmt.Fprintf(os.Stderr, "error: %s", err)
		os.Exit(1)
	}

	fmt.Fprintf(os.Stderr, "================================================================================\n")
	fmt.Fprintf(os.Stderr, "Building firmware binaries\n")
	fmt.Fprintf(os.Stderr, "--------------------------------------------------------------------------------\n")
	binariesDir, err := firmware.Build()
	fmt.Fprintf(os.Stderr, "================================================================================\n")
	if err != nil {
		fmt.Fprintf(os.Stderr, "error: %s", err)
		os.Exit(1)
	}
	defer binariesDir.Cleanup()

	failures := 0
	fmt.Fprintf(os.Stderr, "\n")
	for _, dev := range devices {
		fmt.Fprintf(os.Stderr, "================================================================================\n")
		fmt.Fprintf(os.Stderr, "Running tests for %s\n", dev.Name)
		fmt.Fprintf(os.Stderr, "--------------------------------------------------------------------------------\n")
		err := runDevice(dev, binariesDir.String())
		fmt.Fprintf(os.Stderr, "--------------------------------------------------------------------------------\n")
		if err == nil {
			fmt.Fprintf(os.Stderr, "PASSED (%s)\n", dev.Name)
			fmt.Fprintf(os.Stderr, "================================================================================\n\n")
			continue
		}
		failures++
		fmt.Fprintf(os.Stderr, "FAILED (%s): %s\n", dev.Name, err)
		fmt.Fprintf(os.Stderr, "================================================================================\n\n")
	}
	fmt.Fprintf(os.Stderr, "================================================================================\n")
	if failures == 0 {
		fmt.Fprintf(os.Stderr, "PASSED (%d/%d)\n", len(devices), len(devices))
		return
	}
	fmt.Fprintf(os.Stderr, "FAILED (%d/%d)\n", failures, len(devices))

	// cleanup manually since calling os.Exit prevents deferred functions from running
	binariesDir.Cleanup()
	os.Exit(1)
}
