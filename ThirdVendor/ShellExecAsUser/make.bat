pushd sources
msbuild ShellExecAsUser.sln /t:Build /p:Configuration=Debug /p:Platform=Win32
msbuild ShellExecAsUser.sln /t:Build /p:Configuration=DebugUnicode /p:Platform=Win32
msbuild ShellExecAsUser.sln /t:Build /p:Configuration=Release /p:Platform=Win32
msbuild ShellExecAsUser.sln /t:Build /p:Configuration=ReleaseUnicode /p:Platform=Win32
popd
if not exist dist md dist
copy /y sources\ReleaseUnicode\ShellExecAsUser.dll dist
