SHELL := /bin/bash

.PHONY: debug release relwithdebinfo minsizerel flash flash-release flash-relwithdebinfo flash-minsizerel clean

debug:
	cmake --workflow --preset f446-debug-all

release:
	cmake --workflow --preset f446-release-all

relwithdebinfo:
	cmake --workflow --preset f446-relwithdebinfo-all

minsizerel:
	cmake --workflow --preset f446-minsizerel-all

flash:
	cmake --build --preset f446-build-debug --target flash

flash-release:
	cmake --build --preset f446-build-release --target flash

flash-relwithdebinfo:
	cmake --build --preset f446-build-relwithdebinfo --target flash

flash-minsizerel:
	cmake --build --preset f446-build-minsizerel --target flash

clean:
	rm -rf build/
	rm -f -- -.d

