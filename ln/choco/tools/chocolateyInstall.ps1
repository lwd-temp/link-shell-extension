$packageName = 'ln'
$url = 'https://schinagl.priv.at/nt/ln/save/2924/ln.zip'
$checksum = 'f83e892222610113c5a6eaf731e1f147bbea1dcc8d806d614b67db2a482a4d5f'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/ln/save/2924/ln64.zip'
$checksum64 = 'f3b5d0603a1712c4f8c361407a5c4afac8d81b47f86042684228765f05d87e60'
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