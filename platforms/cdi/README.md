
Building
define environment variables
CDI_DOSBOX_PATH - C:\development\CDI\cdi-sdk-master
CDI_SDK_PATH - C:\development\CDI\DOSBox-0.74-3
CDIEMU_PATH - "C:\development\CDI\cdiemu-0.5.3-beta10\wcdiemu-v053b10.exe"

## requirements
- https://github.com/TwBurn/cdi-sdk
- Use DOSBox to call `vcdmastr.exe` since this is a 16-bit application (see the `cd` target in `build/Makefile`)