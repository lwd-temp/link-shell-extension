$packageName = 'Link Shellextension'
$installerType = 'exe'
$silentArgs = '/S /noredist'
$url = 'https://schinagl.priv.at/nt/hardlinkshellext/save/3933/HardLinkShellExt_win32.exe'
$checksum = 'faf8636fd5f28f60df5158860fb7107e5fa0bd9aedc06c0986b361a7dab7217f'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/hardlinkshellext/save/3933/HardLinkShellExt_X64.exe'
$checksum64 = 'cc60bd4b6808770cd32e748dd4b2bf3f97b9b895c20954fc094d7f5b698a2e9c'
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