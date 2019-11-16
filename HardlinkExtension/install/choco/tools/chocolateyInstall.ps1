$packageName = 'Link Shellextension'
$installerType = 'exe'
$silentArgs = '/S'
$url = 'https://schinagl.priv.at/nt/hardlinkshellext/HardLinkShellExt_win32.exe'
$checksum = '74d21fb9940df5af6d7a661cf6f306e9444a885f5bd367afce92847046214a6f'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/hardlinkshellext/HardLinkShellExt_X64.exe'
$checksum64 = 'f148bb3954b83b6ee269c17fa650f3a0248ba686239b1cff21ff17825b046e23'
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