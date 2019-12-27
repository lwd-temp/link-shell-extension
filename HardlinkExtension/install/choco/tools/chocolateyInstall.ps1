$packageName = 'Link Shellextension'
$installerType = 'exe'
$silentArgs = '/S /noredist'
$url = 'https://schinagl.priv.at/nt/hardlinkshellext/save/3931/HardLinkShellExt_win32.exe'
$checksum = '5975120458cf4acecc0b98057abf10e75a78ef1d5bc2e10b331847a76e508c4f'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/hardlinkshellext/save/3931/HardLinkShellExt_X64.exe'
$checksum64 = '6b72449277601eb7a0c15992edcff7d835d7f99836f0d02a2687856a994a3654'
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