$packageName = 'Link Shellextension'
$installerType = 'exe'
$silentArgs = '/S'
$url = 'https://schinagl.priv.at/nt/hardlinkshellext/HardLinkShellExt_win32.exe'
$checksum = '92537df7c0e262d33959b56f9996d830a797725099ecde51d0db92427c6b04b3'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/hardlinkshellext/HardLinkShellExt_X64.exe'
$checksum64 = '46b98cb6c7864af174edc816545f383c5e786c11e55d15ce9d354ff7a0950b78'
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