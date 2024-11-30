package firmware

import (
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
)

type Dir struct {
	path   string
	isTemp bool
}

func (d *Dir) String() string {
	return d.path
}

func (d *Dir) Cleanup() error {
	if d.isTemp {
		return os.RemoveAll(d.path)
	}
	return nil
}

func Build() (*Dir, error) {
	binariesDir, found := os.LookupEnv("RUNNER_BINARIES_DIR")
	if found {
		fmt.Fprintf(os.Stderr, "Using binaries from %q\n", binariesDir)
		return &Dir{binariesDir, false}, nil
	}

	srcDir, found := os.LookupEnv("RUNNER_SOURCES_DIR")
	if !found {
		if exe, err := os.Executable(); err == nil {
			srcDir = filepath.Join(filepath.Dir(filepath.Dir(exe)), "firmware")
			if _, err := os.Stat(filepath.Join(srcDir, "CMakeLists.txt")); err != nil {
				return nil, errors.New("firmware: failed to guess source or binary directory")
			}
		}
	}

	d, err := os.MkdirTemp("", "runner-cmake")
	if err != nil {
		return nil, err
	}

	cmd := exec.Command("cmake", "-DCMAKE_BUILD_TYPE=Release", "-S", srcDir, "-B", d, "-G", "Ninja")
	cmd.Stderr = os.Stderr
	cmd.Stdout = os.Stdout
	if err := cmd.Run(); err != nil {
		os.RemoveAll(d)
		return nil, err
	}

	cmd = exec.Command("cmake", "--build", d, "--target", "all")
	cmd.Stderr = os.Stderr
	cmd.Stdout = os.Stdout
	if err := cmd.Run(); err != nil {
		os.RemoveAll(d)
		return nil, err
	}
	return &Dir{d, true}, nil
}
