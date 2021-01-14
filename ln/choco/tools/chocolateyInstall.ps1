$packageName = 'ln'
$url = 'https://schinagl.priv.at/nt/ln/save/2931/ln.zip'
$checksum = 'a229fcb3c0bc403cc3d72cabaeb3d89cf8b2b99f03d19c6ab5147bc7b3aa561d'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/ln/save/2931/ln64.zip'
$checksum64 = '9accc61ad084767b99f79e6548da94c3d844775f5e618ba9a04a6192d34f5a32'
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