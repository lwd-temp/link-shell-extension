$packageName = 'Link Shellextension'
$installerType = 'exe'
$silentArgs = '/S /noredist'
$url = 'https://schinagl.priv.at/nt/hardlinkshellext/save/3935/HardLinkShellExt_win32.exe'
$checksum = 'b7e7227e960f025be992c398dafacd03c416adf5210d3fc0ff1d5b5771afdc4b'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/hardlinkshellext/save/3935/HardLinkShellExt_X64.exe'
$checksum64 = 'ca3f26ebf49dc4ea8b5d8c0154acca0de59a8689e5907fe748ffaeaa357ff3a0'
$checksumType64 = 'sha256'
$validExitCodes = @(0)

Install-ChocolateyPackage -PackageName "$packageName" `
                          -FileType "$installerType" `
                          -SilentArgs "$silentArgs" `
                          -Url "$url" `
                          -Url64bit "$url64" `
                          -ValidExitCodes $validExitCodes `
                          -Checksum "$checksum" `
                          -ChecksumType "$checksumType" `
                          -Checksum64 "$checksum64" `
                          -ChecksumType64 "$checksumType64"