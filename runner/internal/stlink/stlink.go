package stlink

// #cgo pkg-config: stlink
// #include <stdlib.h>
// #include <stlink/stlink.h>
// #include <stlink/chipid.h>
// #include <stlink/common_flash.h>
// #include <stlink/version.h>
import "C"

import (
	"errors"
	"fmt"
	"os"
	"strings"
	"unsafe"
)

const (
	MCU_STM32F0 C.uint32_t = 0x0445
	MCU_STM32G4 C.uint32_t = 0x0468
)

var mcus = map[C.uint32_t]string{
	MCU_STM32F0: "NUCLEO-F042K6",
	MCU_STM32G4: "NUCLEO-G431KB",
}

var chipdirs = []string{
	"/usr/share/stlink/chips",
	"/usr/local/share/stlink/chips",
	"/usr/share/stlink/config/chips",
	"/usr/local/share/stlink/config/chips",
}

func Init() error {
	if C.STLINK_VERSION_MAJOR < 1 || C.STLINK_VERSION_MINOR < 8 {
		return fmt.Errorf("stlink: unsupported libstlink version: %d.%d.%d", C.STLINK_VERSION_MAJOR, C.STLINK_VERSION_MINOR, C.STLINK_VERSION_PATCH)
	}

	chipdir := ""
	for _, dir := range chipdirs {
		if _, err := os.Stat(dir); err == nil {
			chipdir = dir
			break
		}
	}
	if chipdir == "" {
		return errors.New("stlink: failed to find chip dir")
	}

	d := C.CString(chipdir)
	defer C.free(unsafe.Pointer(d))
	C.init_chipids(d)

	return nil
}

type Stlink struct {
	Name   string
	ChipID uint32
	Serial string

	dev     *C.stlink_t
	cserial [C.STLINK_SERIAL_BUFFER_SIZE]C.char
	freq    C.int
}

func Enumerate() ([]*Stlink, error) {
	var devicesPtr **C.stlink_t
	n := C.stlink_probe_usb(&devicesPtr, C.CONNECT_NORMAL, 4000)
	if n == 0 {
		return nil, errors.New("stlink: no devices found")
	}
	defer C.stlink_probe_usb_free(&devicesPtr, C.uint32_t(n))
	devices := unsafe.Slice(devicesPtr, n)

	rv := []*Stlink{}
	for _, dev := range devices {
		if dev == nil {
			continue
		}
		for chip_id, name := range mcus {
			if dev.chip_id == chip_id {
				freq := C.int(4000)
				if dev.version.stlink_v == 3 {
					freq = C.int(24000)
				}
				rv = append(rv, &Stlink{
					Name:    name,
					ChipID:  uint32(dev.chip_id),
					Serial:  string(dev.serial[:len(dev.serial)-1]),
					cserial: dev.serial,
					freq:    freq,
				})
				break
			}
		}
	}
	return rv, nil
}

func (s *Stlink) Open() error {
	dev := C.stlink_open_usb(C.UINFO, C.CONNECT_NORMAL, &s.cserial[0], s.freq)
	if dev == nil {
		return errors.New("stlink: failed to open")
	}
	s.dev = dev
	return nil
}

func (s *Stlink) Close() {
	if s.dev != nil {
		C.free(unsafe.Pointer(s.dev))
		s.dev = nil
	}
}

func (s *Stlink) Family() (string, error) {
	if len(s.Name) < 11 {
		return "", errors.New("stlink: invalid name")
	}
	return strings.ToLower(s.Name[7:11]), nil
}

func (s *Stlink) Flash(fname string) error {
	if s.dev == nil {
		return errors.New("stlink: not open")
	}

	defer func() {
		C.stlink_run(s.dev, C.RUN_NORMAL)
		C.stlink_exit_debug_mode(s.dev)
	}()

	if C.stlink_force_debug(s.dev) != 0 {
		return errors.New("stlink: failed to force debug")
	}

	if C.stlink_status(s.dev) != 0 {
		return errors.New("stlink: failed to get status")
	}

	cfname := C.CString(fname)
	defer C.free(unsafe.Pointer(cfname))

	var (
		mem  *C.uint8_t = nil
		size C.uint32_t = 0
		addr C.uint32_t = 0
	)
	if C.stlink_parse_ihex(cfname, C.stlink_get_erased_pattern(s.dev), &mem, &size, &addr) == -1 {
		return errors.New("stlink: failed to parse ihex")
	}

	if C.stlink_mwrite_flash(s.dev, mem, size, addr) == -1 {
		return errors.New("stlink: failed to write flash")
	}

	C.stlink_reset(s.dev, C.RESET_AUTO)
	return nil
}
