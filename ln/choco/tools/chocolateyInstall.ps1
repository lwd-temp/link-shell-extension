$packageName = 'ln'
$url = 'https://schinagl.priv.at/nt/ln/save/2930/ln.zip'
$checksum = 'd9d4f7a2f1dec92b201a042b6efc600ce06e30aaef53f3547d2be20a0f881646'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/ln/save/2930/ln64.zip'
$checksum64 = '0316be3abdca74afb3a2bbd995edbaef155c4adb6bcb048706ccab66ec9f087e'
$checksumType64 = 'sha256'
$toolsDir = "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"

Install-ChocolateyZipPackage -PackageName "$packageName" `
                             -Url "$url" `
                             -UnzipLocation "$toolsDir" `
                             -Url64bit "$url64" `
                             -Checksum "$checksum" `
                             -ChecksumType "$checksumType" `
                             -Checksum64 "$checksum64" `
                             -ChecksumType64 "$checksumType64"