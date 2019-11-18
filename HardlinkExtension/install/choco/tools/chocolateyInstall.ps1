$packageName = 'Link Shellextension'
$installerType = 'exe'
$silentArgs = '/S /noredist'
$url = 'https://schinagl.priv.at/nt/hardlinkshellext/save/3930/HardLinkShellExt_win32.exe'
$checksum = '2e62d4a7d4cdd4d36f67a82cd64d1187cb6411fdb5863c79faa09c7c751390c1'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/hardlinkshellext/save/3930/HardLinkShellExt_X64.exe'
$checksum64 = 'dcaa5ef7c34846d1a41d6dc67d4887c86e2351c2a6561363e86e280a872d3f97'
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