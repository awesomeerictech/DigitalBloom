# Minimal: force CMake to use llvm-* tools from a fixed NDK bin dir and cache them (FORCE)
# Set your NDK bin directory here (no scanning). Adjust host dir if needed.
set(_NDK_BIN_DIR "C:/androidsdk/ndk/29.0.14206865/toolchains/llvm/prebuilt/windows-x86_64/bin" CACHE PATH "Android NDK bin dir (fixed)")

set(CMAKE_OBJCOPY "${_NDK_BIN_DIR}/llvm-objcopy.exe" CACHE FILEPATH "objcopy tool" FORCE)
set(CMAKE_RANLIB "${_NDK_BIN_DIR}/llvm-ranlib.exe" CACHE FILEPATH "ranlib tool" FORCE)
set(CMAKE_AR "${_NDK_BIN_DIR}/llvm-ar.exe" CACHE FILEPATH "Archive tool" FORCE)
set(CMAKE_NM "${_NDK_BIN_DIR}/llvm-nm.exe" CACHE FILEPATH "nm tool" FORCE)
set(CMAKE_STRIP "${_NDK_BIN_DIR}/llvm-strip.exe" CACHE FILEPATH "strip tool" FORCE)

#[[
# In your my-android-toolchain.cmake file

# Define the path where your llvm tools are located in the Android NDK
# You MUST replace this with the actual path on your Windows machine
set(LLVM_TOOLS_BIN_DIR "C:/androidsdk/ndk/29.0.14206865/toolchains/llvm/prebuilt/windows-x86_64/bin") 

set(_tools_map
  "CMAKE_AR;llvm-ar.exe"
  "CMAKE_RANLIB;llvm-ranlib.exe"
  "CMAKE_NM;llvm-nm.exe"
  "CMAKE_STRIP;llvm-strip.exe"
  "CMAKE_OBJDUMP;llvm-objdump.exe"
  "CMAKE_OBJCOPY;llvm-objcopy.exe"
)

foreach(_tool_pair IN LISTS _tools_map)
  string(REPLACE ";" " " _tool_vars "${_tool_pair}")
  list(POP_FRONT _tool_vars _cmake_var)
  list(POP_FRONT _tool_vars _tool_name)

  # Explicitly set the variable using the full path and FORCE it
  set(${_cmake_var} "${LLVM_TOOLS_BIN_DIR}/${_tool_name}" CACHE FILEPATH "Path to the ${_tool_name} tool" FORCE)
endforeach()
]]