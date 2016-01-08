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
all: test

# Configures X3D, the unit tests, and the manual tests
config:
	@tput setaf 2
	@echo Configuring X3D
	@tput setaf 7
	@mkdir -p build/X3D
	@cd build/X3D && cmake ../../src
	
	@tput setaf 2
	@echo Configuring Unit Tests
	@tput setaf 7
	@mkdir -p build/unit
	@cd build/unit && cmake ../../test/unit
	
	@tput setaf 2
	@echo Configuring Manual Tests
	@tput setaf 7
	@mkdir -p build/manual
	@cd build/manual && cmake ../../test/pc/test-pc
	
# Builds X3D
x3d:
	@cd build/X3D && make --no-print-directory
	
# Builds X3D and builds/runs the unit tests
test: x3d
	@cd build/unit && rm -f ./unit && make --no-print-directory && ./unit

# Builds X3D and builds/runs the manual tests
test-manual: x3d
	@cd build/manual && make --no-print-directory && ./test-pc

# Builds the documentation
docs:
	@cd docs && doxygen Doxyfile


concat-build:
	find "src/C" -type f -name "*.c" -exec cat {} \; >output.txt
