$packageName = 'dupemerge'
$url = 'https://schinagl.priv.at/nt/dupemerge/save/1104/dupemerge.zip'
$checksum = 'a5af1a0cf95f7e6fcb805a7447d09b63f933ecc35c75360fde7840cac31090af'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/dupemerge/save/1104/dupemerge64.zip'
$checksum64 = '363d756ec91d6558b4956ea80464b264202d8c2be1706e06faeac86bac0899ea'
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