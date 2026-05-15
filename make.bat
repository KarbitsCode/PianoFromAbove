if not exist ThirdVendor\protobuf-2.5.0\vsprojects\Release (
  pushd ThirdVendor\protobuf-2.5.0\vsprojects
  cmd /c make.bat
  popd
)
msbuild PianoFromAbove.sln /t:Build /p:Configuration=Release /p:Platform=Win32
msbuild PianoFromAbove.sln /t:Build /p:Configuration=Release /p:Platform=x64
makensis /V4 PianoFromAbove.nsi
