$packageName = 'dupemerge'
$url = 'https://schinagl.priv.at/nt/dupemerge/dupemerge.zip'
$checksum = 'fce525818990ff54f384459310807fcf44bd83604aa019bee83e6a9c384b05bb'
$checksumType = 'sha256'
$url64 = 'https://schinagl.priv.at/nt/dupemerge/dupemerge64.zip'
$checksum64 = '7a27b83f4e34ecaff90f0b9f610570f04141f39a2cdebe3aa123d0fb38467a18'
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