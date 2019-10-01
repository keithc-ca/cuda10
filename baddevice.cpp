#include <stdio.h>
#include <stdlib.h>

#if defined(WIN32)
# include <windows.h>
  typedef HMODULE Handle_t;
#else /* defined(WIN32) */
# include <dlfcn.h>
  typedef void * Handle_t;
#endif /* defined(WIN32) */

enum cudaError_t
{
	cudaSuccess = 0
  , cudaErrorInvalidDevice = 10
};

class CudaRuntime
{
private:
	Handle_t _handle;
	cudaError_t (* _getDeviceCount)(int * count);
	cudaError_t (* _setDevice)(int deviceId);

	void * findSymbol(const char * function);

public:
	CudaRuntime() : _handle(NULL), _getDeviceCount(NULL), _setDevice(NULL) {}

	bool open(const char *libname);
	void close();

	cudaError_t getDeviceCount(int * count) {
		return _getDeviceCount(count);
	}

	cudaError_t setDevice(int deviceId) {
		return _setDevice(deviceId);
	}
};

void * CudaRuntime::findSymbol(const char * function)
{
#if defined(WIN32)
	return (void *) (uintptr_t) GetProcAddress(_handle, function);
#else /* defined(WIN32) */
	return dlsym(_handle, function);
#endif /* defined(WIN32) */
}

bool CudaRuntime::open(const char *libname)
{
#if defined(WIN32)
	_handle = LoadLibrary(libname);
#else /* defined(WIN32) */
	_handle = dlopen(libname, RTLD_LAZY);
#endif /* defined(WIN32) */

	if (NULL != _handle) {
		_getDeviceCount = (cudaError_t (*)(int *)) findSymbol("cudaGetDeviceCount");
		_setDevice = (cudaError_t (*)(int)) findSymbol("cudaSetDevice");

		if (_getDeviceCount && _setDevice) {
			return true;
		}
	}

	return false;
}

void CudaRuntime::close()
{
	if (NULL != _handle) {
#if defined(WIN32)
		FreeLibrary(_handle);
#else /* defined(WIN32) */
		dlclose(_handle);
#endif /* defined(WIN32) */
		_handle = NULL;
		_getDeviceCount = NULL;
		_setDevice = NULL;
	}
}

#if defined(WIN32)
static const char libraryName[] = "cudart64_101.dll";
#else /* defined(WIN32) */
static const char libraryName[] = "libcudart.so";
#endif /* defined(WIN32) */

int main(int argc, const char * const * argv)
{
	int result = EXIT_FAILURE;
	const char *libName = argc > 1 ? argv[1] : libraryName;
	int deviceCount = 0;
	CudaRuntime runtime;

	if (!runtime.open(libName)) {
		printf("Failed to open CUDA runtime\n");
	} else {
		cudaError_t error = runtime.getDeviceCount(&deviceCount);

		if (cudaSuccess != error) {
			printf("cudaGetDeviceCount returned %d\n", (int)error);
		} else if (0 == deviceCount) {
			printf("No CUDA devices found.\n");
			result = EXIT_SUCCESS;
		} else {
			printf("Detected %d CUDA Capable device(s)\n", deviceCount);

			error = runtime.setDevice(deviceCount);

			if (cudaErrorInvalidDevice == error) {
				printf("Expected failure of cudaSetDevice() with a bad device id\n");
				result = EXIT_SUCCESS;
			} else if (cudaSuccess == error) {
				printf("Unexpected success of cudaSetDevice() with a bad device id\n");
			} else {
				printf("Unexpected failure of cudaSetDevice() with a bad device id: %d\n", (int)error);
			}
		}
	}

	printf("Result = %s\n", (EXIT_SUCCESS == result) ? "PASS" : "FAIL");

	return result;
}
