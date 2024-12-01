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

type stlinkConnection C.stlink_t

func newStlinkConnection(serial *C.char, freq C.int) (*stlinkConnection, error) {
	dev := C.stlink_open_usb(C.UINFO, C.CONNECT_NORMAL, serial, freq)
	if dev == nil {
		return nil, errors.New("stlink: failed to open")
	}

	if C.stlink_force_debug(dev) != 0 {
		C.stlink_close(dev)
		return nil, errors.New("stlink: failed to force debug")
	}

	if C.stlink_status(dev) != 0 {
		C.stlink_run(dev, C.RUN_NORMAL)
		C.stlink_exit_debug_mode(dev)
		C.stlink_close(dev)
		return nil, errors.New("stlink: failed to get status")
	}

	return (*stlinkConnection)(dev), nil
}

func (s *stlinkConnection) Close(run bool) error {
	if run {
		if C.stlink_run((*C.stlink_t)(s), C.RUN_NORMAL) != 0 {
			return errors.New("stlink: failed to run")
		}
	}

	if C.stlink_exit_debug_mode((*C.stlink_t)(s)) != 0 {
		return errors.New("stlink: failed to exit debug mode")
	}

	C.stlink_close((*C.stlink_t)(s))
	return nil
}

type Stlink struct {
	Name   string
	ChipID uint32
	Serial string
	Family string

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
				if len(name) < 11 {
					return nil, errors.New("stlink: invalid name")
				}

				freq := C.int(4000)
				if dev.version.stlink_v == 3 {
					freq = C.int(24000)
				}

				rv = append(rv, &Stlink{
					Name:   name,
					ChipID: uint32(dev.chip_id),
					Serial: string(dev.serial[:len(dev.serial)-1]),
					Family: strings.ToLower(name[7:11]),

					cserial: dev.serial,
					freq:    freq,
				})
				break
			}
		}
	}
	return rv, nil
}

func (s *Stlink) Flash(fname string) error {
	c, err := newStlinkConnection(&s.cserial[0], s.freq)
	if err != nil {
		return err
	}
	defer c.Close(true)

	cfname := C.CString(fname)
	defer C.free(unsafe.Pointer(cfname))

	var (
		mem  *C.uint8_t = nil
		size C.uint32_t = 0
		addr C.uint32_t = 0
	)
	if C.stlink_parse_ihex(cfname, C.stlink_get_erased_pattern((*C.stlink_t)(c)), &mem, &size, &addr) == -1 {
		return errors.New("stlink: failed to parse ihex")
	}

	if C.stlink_mwrite_flash((*C.stlink_t)(c), mem, size, addr) == -1 {
		return errors.New("stlink: failed to write flash")
	}

	C.stlink_reset((*C.stlink_t)(c), C.RESET_AUTO)
	return nil
}

func (s *Stlink) Erase() error {
	c, err := newStlinkConnection(&s.cserial[0], s.freq)
	if err != nil {
		return err
	}
	defer c.Close(true)

	if C.stlink_erase_flash_mass((*C.stlink_t)(c)) != 0 {
		return errors.New("stlink: failed to erase device")
	}

	C.stlink_reset((*C.stlink_t)(c), C.RESET_AUTO)
	return nil
}

func (s *Stlink) Halt() error {
	c, err := newStlinkConnection(&s.cserial[0], s.freq)
	if err != nil {
		return err
	}
	defer c.Close(false)

	if C.stlink_reset((*C.stlink_t)(c), C.RESET_SOFT_AND_HALT) != 0 {
		return errors.New("stlink: failed to reset and halt device")
	}
	return nil
}
