$packageName = 'Link Shellextension'
$installerType = 'exe'
$silentArgs = '/S /noredist'
$url = 'https://schinagl.priv.at/nt/hardlinkshellext/save/3932/HardLinkShellExt_win32.exe'
$checksum = '0db6aeeb2e64e76e970b0d50ce6f1ea0e065d717ccf4173e34ce29f2a4bc04f4'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/hardlinkshellext/save/3932/HardLinkShellExt_X64.exe'
$checksum64 = '328b9ce1257a2e148faa4a611a11f48e352cbb663dd9c1e9240e1a4f9d72ab47'
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