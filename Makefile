PROJECT_NAME=ShiftMe

BUILD_FOLDER=$(PROJECT_NAME)/Builds/LinuxMakefile

all:
	cd $(BUILD_FOLDER) && make

VST:
	cd $(BUILD_FOLDER) && make VST3

Standalone:
	cd $(BUILD_FOLDER) && make Standalone

$(BUILD_FOLDER)/build/$(PROJECT_NAME):
	cd $(BUILD_FOLDER) && make Standalone

test: $(BUILD_FOLDER)/build/$(PROJECT_NAME)
	./$(BUILD_FOLDER)/build/$(PROJECT_NAME)

clean:
	cd $(BUILD_FOLDER) && make clean
