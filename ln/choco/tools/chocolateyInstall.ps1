$packageName = 'ln'
$url = 'https://schinagl.priv.at/nt/ln/save/2933/ln.zip'
$checksum = 'be87e1593f40f6d37111655cc6e903a0644bcffe1282a0fee18ff90052c3b2bd'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/ln/save/2933/ln64.zip'
$checksum64 = '27fadef5143aa76e3840b96f2626bb9d6d511bdd889979513d62d3994bde39d2'
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