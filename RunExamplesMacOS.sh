#!/bin/sh
BUILD_DIR="build/build"

if [ "$#" -eq 1 ]; then
    BUILD_DIR=$1
elif [ -d "build/build/Example_HelloTriangle.app" ]; then
    BUILD_DIR="build/build"
elif [ -d "bin/x64/MacOS/Example_HelloTriangle.app" ]; then
    BUILD_DIR="bin/x64/MacOS"
else
    echo "error: build folder not found: $BUILD_DIR"
    exit 1
fi

PS3='select example: '
options=(
    "Hello Triangle"
    "Tessellation"
    "Texturing"
    "Render Target"
    "Buffer Array"
    "Stream Output"
    "Post Processing"
    "Shadow Mapping"
    "Stencil Buffer"
    "Volume Rendering"
    "Compute Shader"
)
select opt in "${options[@]}"
do
    case $opt in
    "${options[0]}")
        (cd examples/Cpp/HelloTriangle; ../../../$BUILD_DIR/Example_HelloTriangle.app/Contents/MacOS/Example_HelloTriangle)
        ;;
    "${options[1]}")
        (cd examples/Cpp/Tessellation; ../../../$BUILD_DIR/Example_Tessellation.app/Contents/MacOS/Example_Tessellation)
        ;;
    "${options[2]}")
        (cd examples/Cpp/Texturing; ../../../$BUILD_DIR/Example_Texturing.app/Contents/MacOS/Example_Texturing)
        ;;
    "${options[3]}")
        (cd examples/Cpp/RenderTarget; ../../../$BUILD_DIR/Example_RenderTarget.app/Contents/MacOS/Example_RenderTarget)
        ;;
    "${options[4]}")
        (cd examples/Cpp/BufferArray; ../../../$BUILD_DIR/Example_BufferArray.app/Contents/MacOS/Example_BufferArray)
        ;;
    "${options[5]}")
        (cd examples/Cpp/StreamOutput; ../../../$BUILD_DIR/Example_StreamOutput.app/Contents/MacOS/Example_StreamOutput)
        ;;
    "${options[6]}")
        (cd examples/Cpp/PostProcessing; ../../../$BUILD_DIR/Example_PostProcessing.app/Contents/MacOS/Example_PostProcessing)
        ;;
    "${options[7]}")
        (cd examples/Cpp/ShadowMapping; ../../../$BUILD_DIR/Example_ShadowMapping.app/Contents/MacOS/Example_ShadowMapping)
        ;;
    "${options[8]}")
        (cd examples/Cpp/StencilBuffer; ../../../$BUILD_DIR/Example_StencilBuffer.app/Contents/MacOS/Example_StencilBuffer)
        ;;
    "${options[9]}")
        (cd examples/Cpp/VolumeRendering; ../../../$BUILD_DIR/Example_VolumeRendering.app/Contents/MacOS/Example_VolumeRendering)
        ;;
    "${options[10]}")
        (cd examples/Cpp/ComputeShader; ../../../$BUILD_DIR/Example_ComputeShader.app/Contents/MacOS/Example_ComputeShader)
        ;;
    *)
        echo "invalid selection"
        ;;
    esac
done
