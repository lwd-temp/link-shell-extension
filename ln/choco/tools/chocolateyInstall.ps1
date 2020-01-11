$packageName = 'ln'
$url = 'https://schinagl.priv.at/nt/ln/save/2925/ln.zip'
$checksum = '652b374466131f060975e0edae23c7ba2b685e7d94ca22a00316725d290ab124'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/ln/save/2925/ln64.zip'
$checksum64 = '642db276a082a0e966378d68d15b633891d72bf194d6fabb91efcb57a8a67fa8'
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