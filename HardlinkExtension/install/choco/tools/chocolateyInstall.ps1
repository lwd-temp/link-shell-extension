$packageName = 'Link Shellextension'
$installerType = 'exe'
$silentArgs = '/S /noredist'
$url = 'https://schinagl.priv.at/nt/hardlinkshellext/save/3934/HardLinkShellExt_win32.exe'
$checksum = '1353122044bf8613bdd9a557b1e003724e579bde410a87f5e2e831c4ecadef68'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/hardlinkshellext/save/3934/HardLinkShellExt_X64.exe'
$checksum64 = '032db48f2c84011dd65d141cc52f00528883e7257a946f77143a957f5ca3fe9a'
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