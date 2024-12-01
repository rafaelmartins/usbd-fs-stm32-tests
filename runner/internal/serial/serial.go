package serial

// #cgo CFLAGS: -I../../../common
// #include <serial-protocol.h>
import "C"

import (
	"errors"
	"fmt"
	"os"
	"strings"

	"go.bug.st/serial"
)

type Serial struct {
	port serial.Port
	sn   string
}

func New(sn string) (*Serial, error) {
	ports, err := os.ReadDir("/dev/serial/by-id")
	if err != nil {
		return nil, err
	}

	portName := ""
	for _, p := range ports {
		if strings.Contains(p.Name(), sn) {
			portName = p.Name()
			break
		}
	}
	if portName == "" {
		return nil, errors.New("serial: no port found")
	}

	port, err := serial.Open("/dev/serial/by-id/"+portName, &serial.Mode{
		BaudRate: 115200,
	})
	if err != nil {
		return nil, err
	}
	return &Serial{
		port: port,
		sn:   sn,
	}, nil
}

func (s *Serial) Close() error {
	if s.port != nil {
		return s.port.Close()
	}
	return nil
}

func (s *Serial) sendCommand(cmd byte, val byte) (byte, error) {
	n, err := s.port.Write([]byte{(cmd << 4) | (val & 0xf)})
	if err != nil {
		return 0, err
	}
	if n != 1 {
		return 0, fmt.Errorf("serial: failed to write command: %#x %#x", cmd, val)
	}

	b := make([]byte, 1)
	n, err = s.port.Read(b)
	if err != nil {
		return 0, err
	}
	if n == 0 {
		return 0, fmt.Errorf("serial: returned EOF")
	}

	rcmd, rval := b[0]>>4, b[0]&0x0f
	if rcmd != cmd {
		return 0, fmt.Errorf("serial: bad response, expected command %#x, got %#x", cmd, rcmd)
	}
	return rval, nil
}

func (s *Serial) ConfigureTest(testID byte) error {
	val, err := s.sendCommand(C.PROTO_CMD_CONFIGURE_TEST, testID)
	if err != nil {
		return err
	}

	switch val {
	case C.PROTO_REPLY_CONFIGURE_TEST_OK:
		return nil
	case C.PROTO_REPLY_CONFIGURE_TEST_INVALID_ID:
		return fmt.Errorf("serial: failed to configure test: invalid id: %d", testID)
	case C.PROTO_REPLY_CONFIGURE_TEST_FAILED:
		return fmt.Errorf("serial: failed to configure test")
	default:
		return fmt.Errorf("serial: failed to configure test: unknown error: %#x", val)
	}
}

func (s *Serial) GetTestCount() (byte, error) {
	return s.sendCommand(C.PROTO_CMD_GET_TEST_COUNT, 0)
}

func (s *Serial) GetTestResult() error {
	val, err := s.sendCommand(C.PROTO_CMD_GET_TEST_RESULT, 0)
	if err != nil {
		return err
	}

	switch val {
	case C.PROTO_REPLY_GET_TEST_RESULT_PASSED:
		return nil
	case C.PROTO_REPLY_GET_TEST_RESULT_FAILED:
		return fmt.Errorf("serial: test failed")
	default:
		return fmt.Errorf("serial: failed to get test result: unknown error: %#x", val)
	}
}

func (s *Serial) Sync() error {
	val, err := s.sendCommand(C.PROTO_CMD_SYNC, C.PROTO_CMD_SYNC_VAL_SYNC)
	if err != nil {
		return err
	}
	if val != C.PROTO_CMD_SYNC_VAL_SYNC {
		return fmt.Errorf("serial: bad sync: %#x", val)
	}
	return nil
}
