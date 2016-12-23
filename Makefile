# This file is part of X3D.
#
# X3D is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# X3D is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with X3D. If not, see <http:#www.gnu.org/licenses/>.

.PHONY: all
.PHONY: test
.PHONY: test-manual
.PHONY: x3d
.PHONY: config
.PHONY: docs

# Builds X3D and builds/runs the unit tests
all: x3d

# Configures X3D, the unit tests, and the manual tests
config:
	#@tput setaf 2
	@echo Configuring X3D
	#@tput setaf 7
	@mkdir -p build/X3D
	@cd build/X3D && cmake ../../src -DX3D_TARGET=pc
	
	#@tput setaf 2
	@echo Configuring Unit Tests
	@tput setaf 7
	@mkdir -p build/unit
	@cd build/unit && cmake ../../test/unit -DX3D_TARGET=pc
	
	#@tput setaf 2
	@echo Configuring Manual Tests
	#@tput setaf 7
	@mkdir -p build/manual
	@cd build/manual && cmake ../../test/pc/test-pc -DX3D_TARGET=pc

config-68k:
	@echo "Configuring X3D-68k"
	@mkdir -p build/X3D-68k
	@cd build/X3D-68k && cmake ../../src -DX3D_TARGET=68k -DX3D_SUBTARGET=ti89

config-nspire:
	@echo "Configuring nX3D"
	@mkdir -p build/nX3D
	@cd build/nX3D && cmake ../../src -DX3D_TARGET=nspire
	
	
config-68k-test:
	#@tput setaf 2
	@echo Configuring Manual Tests for 68k
	#@tput setaf 7
	@mkdir -p build/manual-68k
	@cd build/manual-68k && cmake ../../test/pc/test-pc -DX3D_TARGET=68k -DX3D_SUBTARGET=ti89

config-nspire-test:
	#@tput setaf 2
	@echo Configuring Manual Tests for nspire
	#@tput setaf 7
	@mkdir -p build/manual-nspire
	@cd build/manual-nspire && cmake ../../test/pc/test-pc -DX3D_TARGET=nspire

# Builds X3D
x3d:
	@cd build/X3D && make --no-print-directory

x3d-68k:
	@cd build/X3D-68k && make --no-print-directory

nx3d:
	@cd build/nX3D && make --no-print-directory

# Builds X3D and builds/runs the unit tests
test: x3d
	@cd build/unit && rm -f ./unit && make --no-print-directory && ./unit

# Builds X3D and builds/runs the manual tests
test-manual: x3d
	@cd build/manual && make clean --no-print-directory && make --no-print-directory && ./test-pc
	
test-manual-68k: x3d-68k
	@cd build/manual-68k && make --no-print-directory

test-manual-nspire: nx3d
	@cd build/manual-nspire && make --no-print-directory


install:
	@cd build/X3D && sudo make install

analyze:
	@export CC=ccc-analyzer
	@export CCC_CC=clang
	@export LD=clang
	@export CCC_ANALYZER_VERBOSE=1
	@cd build/X3D && make clean --no-print-directory && make --no-print-directory

