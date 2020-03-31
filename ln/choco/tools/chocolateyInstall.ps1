$packageName = 'ln'
$url = 'https://schinagl.priv.at/nt/ln/save/2928/ln.zip'
$checksum = 'e4ba6d09af236479bae055ac0200931440ad189670af1403c06fad7d5bbe1ae5'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/ln/save/2928/ln64.zip'
$checksum64 = 'a9fefe12861792e2e2f3b3691ea5e015521bb932b06b314383dc8ca0fe7a05aa'
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