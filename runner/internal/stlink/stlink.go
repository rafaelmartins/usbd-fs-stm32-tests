package stlink

// #cgo pkg-config: stlink
// #include <stdlib.h>
// #include <stlink/stlink.h>
import "C"

import (
	"errors"
	"unsafe"
)

const (
	MCU_STM32F0 C.uint32_t = 0x0445
	MCU_STM32G4 C.uint32_t = 0x0468
)

var (
	mcus = map[C.uint32_t]string{
		MCU_STM32F0: "NUCLEO-F042K6",
		MCU_STM32G4: "NUCLEO-G431KB",
	}
)

type Stlink struct {
	dev    *C.stlink_t
	Name   string
	ChipID uint32
}

func Enumerate() ([]*Stlink, error) {
	var devicesPtr **C.stlink_t
	n := C.stlink_probe_usb(&devicesPtr, C.CONNECT_NORMAL, 24000)
	if n == 0 {
		return nil, errors.New("stlink: no devices found")
	}
	devices := unsafe.Slice(devicesPtr, n)

	rv := []*Stlink{}
	for _, dev := range devices {
		for chip_id, name := range mcus {
			if dev.chip_id == chip_id {
				rv = append(rv, &Stlink{
					dev:    dev,
					Name:   name,
					ChipID: uint32(chip_id),
				})
				break
			} else {
				C.free(unsafe.Pointer(dev))
			}
		}
	}

	return rv, nil
}

func (s *Stlink) Close() {
	if s.dev != nil {
		C.free(unsafe.Pointer(s.dev))
		s.dev = nil
	}
}
