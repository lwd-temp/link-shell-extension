$packageName = 'ln'
$url = 'https://schinagl.priv.at/nt/ln/save/2934/ln.zip'
$checksum = '30ca81c2b75f9a8b0095aaf825dac02deb336c58f608b0649ec84f20d128399c'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/ln/save/2934/ln64.zip'
$checksum64 = '9ac19f84846c2571fcf1017e88a7b58959e739620b6628f0cf7a1a2299b28ba2'
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