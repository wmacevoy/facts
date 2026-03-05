$ErrorActionPreference = 'Stop'

$version = '1.3.0'
$url = "https://github.com/wmacevoy/facts/archive/refs/tags/v$version.zip"

$toolsDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$buildDir = Join-Path $toolsDir 'build'
$srcDir = Join-Path $toolsDir "facts-$version"

# Download and extract
$zipFile = Join-Path $toolsDir "facts-$version.zip"
Get-ChocolateyWebFile -PackageName 'libfacts' -FileFullPath $zipFile -Url $url
Get-ChocolateyUnzip -FileFullPath $zipFile -Destination $toolsDir

# Build with CMake
$installPrefix = Join-Path $env:ChocolateyInstall 'lib' 'libfacts'
cmake -S $srcDir -B $buildDir `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_INSTALL_PREFIX="$installPrefix"
cmake --build $buildDir --config Release
cmake --install $buildDir --config Release

# Clean up build artifacts
Remove-Item -Recurse -Force $buildDir, $srcDir, $zipFile
