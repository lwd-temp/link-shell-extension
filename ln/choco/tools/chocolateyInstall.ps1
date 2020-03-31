$packageName = 'ln'
$url = 'https://schinagl.priv.at/nt/ln/save/2927/ln.zip'
$checksum = 'e9a97d0edc6a76163f5a58d3dcb0203aa700320ac5d71f52f822b3b03b87ceb8'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/ln/save/2927/ln64.zip'
$checksum64 = '1a990b523a6c4503b21c654d7f8e1ebfd912392cd9edf26a36fef9718b05d635'
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