PROJECT_NAME=editor
PROJECT_VERSION=1.0
OUTPUT=./bin
OUTPUT_DEBUG=${OUTPUT}/debug
OUTPUT_RELEASE=${OUTPUT}/release

all: debug

configure: BUILD_DIR=${OUTPUT_DEBUG}
configure: BUILD_TYPE=Debug
configure: cmake-configure

configure-release: BUILD_DIR=${OUTPUT_RELEASE}
configure-release: BUILD_TYPE=Release
configure-release: cmake-configure

build: BUILD_DIR=${OUTPUT_DEBUG}
build: cmake-build

build-release: BUILD_DIR=${OUTPUT_RELEASE}
build-release: cmake-build

run:
	@${OUTPUT_DEBUG}/${PROJECT_NAME}

run-release:
	@${OUTPUT_RELEASE}/${PROJECT_NAME}

clean:
	@rm -rf ${OUTPUT}

debug: clean configure build

release: clean configure-release build-release

cmake-build:
	@cd ${BUILD_DIR} && make

cmake-configure:
	@PROJECT_NAME=${PROJECT_NAME} \
		PROJECT_VERSION=${PROJECT_VERSION} \
		OUTPUT_RELEASE=${OUTPUT_RELEASE} \
		cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -S . -B ${BUILD_DIR}
